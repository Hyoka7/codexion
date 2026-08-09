#!/bin/sh

set -eu

binary=${CODEXION_BINARY:-./codexion}
repeat_count=${BURNOUT_TEST_REPEATS:-10}
tolerance_ms=10
tmp_dir=$(mktemp -d)

trap 'rm -rf "$tmp_dir"' EXIT HUP INT TERM

validate_log()
{
	log_file=$1
	burnout_ms=$2
	case_name=$3
	attempt=$4
	awk -v burnout_ms="$burnout_ms" -v tolerance_ms="$tolerance_ms" \
		-v case_name="$case_name" -v attempt="$attempt" '
		function fail(message)
		{
			printf "%s attempt %d: %s\n", case_name, attempt, message > "/dev/stderr"
			exit 1
		}
		{
			if (burn_seen)
				fail("a log was emitted after burnout")
			if ($3 == "is" && $4 == "compiling")
			{
				state[$2] = "compiling"
				compile_time[$2] = $1
				has_compiled[$2] = 1
			}
			else if ($3 == "is" && $4 == "debugging")
				state[$2] = "debugging"
			else if ($3 == "is" && $4 == "refactoring")
				state[$2] = "refactoring"
			else if ($3 == "burned" && $4 == "out")
			{
				burn_seen = 1
				burn_count++
				burn_time = $1
				burned_coder = $2
			}
		}
		END {
			if (burn_count != 1)
				fail("expected exactly one burnout log")
			expected = burnout_ms
			if (has_compiled[burned_coder])
				expected += compile_time[burned_coder]
			error = burn_time - expected
			if (error < 0)
				error = -error
			if (error > tolerance_ms)
				fail("burnout timestamp differs from its deadline by more than 10 ms")
			if (state[burned_coder] == "")
				state[burned_coder] = "waiting"
			print state[burned_coder]
		}
	' "$log_file"
}

run_case()
{
	case_name=$1
	expected_state=$2
	burnout_ms=$3
	max_attempts=$4
	shift 4
	attempt=0
	matched=0
	while [ "$attempt" -lt "$max_attempts" ] && [ "$matched" -lt "$repeat_count" ]
	do
		attempt=$((attempt + 1))
		log_file="$tmp_dir/$case_name-$attempt.log"
		if ! timeout 5s "$binary" "$@" > "$log_file"
		then
			echo "$case_name attempt $attempt: simulation failed or timed out" >&2
			return 1
		fi
		if ! observed_state=$(validate_log "$log_file" "$burnout_ms" \
				"$case_name" "$attempt")
		then
			return 1
		fi
		if [ "$expected_state" = any ] || [ "$observed_state" = "$expected_state" ]
		then
			matched=$((matched + 1))
		fi
	done
	if [ "$matched" -ne "$repeat_count" ]
	then
		echo "$case_name: observed $expected_state only $matched times in $attempt attempts" >&2
		return 1
	fi
	printf '%s: %d timing checks passed\n' "$case_name" "$matched"
}

case "$repeat_count" in
	''|*[!0-9]*|0)
		echo "BURNOUT_TEST_REPEATS must be a positive integer" >&2
		exit 1
		;;
esac

max_variable_attempts=$((repeat_count * 10))

run_case one-coder-waiting waiting 100 "$repeat_count" \
	1 100 10 10 10 1 0 fifo
run_case multiple-coder-waiting waiting 80 "$max_variable_attempts" \
	5 80 150 10 10 2 20 fifo
run_case compiling compiling 60 "$max_variable_attempts" \
	2 60 150 0 0 1 0 fifo
run_case debugging debugging 80 "$max_variable_attempts" \
	3 80 5 200 0 2 0 fifo

echo "All burnout timing checks passed within ${tolerance_ms} ms"
