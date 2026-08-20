*This project has been created as part of the 42 curriculum by hfujisad.*

# Codexion

## Description

Codexion is a concurrent simulation written in C using POSIX threads. It
models a group of coders competing for a ring of shared USB dongles. Each
coder needs the two adjacent dongles to compile, releases them after the
compile phase, and then debugs and refactors before requesting them again.

The goal is to coordinate access to the dongles while respecting:

- FIFO or EDF arbitration for every dongle;
- a mandatory cooldown after each dongle is released;
- a burnout deadline for every coder;
- fairness, liveness, and precise serialized logging;
- safe initialization, shutdown, and resource cleanup.

The simulation stops when every coder has completed the required number of
compiles, or when one coder burns out.

## Instructions

### Requirements

- A POSIX-compatible operating system
- A C compiler supporting POSIX threads
- `make`

### Compilation

```bash
make
```

The executable is created as `codexion`.

Other available targets are:

```bash
make clean   # Remove object files
make fclean  # Remove object files and the executable
make re      # Rebuild the project from scratch
```

### Execution

```bash
./codexion number_of_coders time_to_burnout time_to_compile \
  time_to_debug time_to_refactor number_of_compiles_required \
  dongle_cooldown scheduler
```

All time values are expressed in milliseconds.

| Argument | Description |
|---|---|
| `number_of_coders` | Number of coder threads and dongles. Must be greater than zero. |
| `time_to_burnout` | Maximum time between the start of two consecutive compile phases. |
| `time_to_compile` | Time spent compiling while holding two dongles. |
| `time_to_debug` | Time spent debugging after a compile. |
| `time_to_refactor` | Time spent refactoring after debugging. |
| `number_of_compiles_required` | Number of compiles every coder must complete. |
| `dongle_cooldown` | Time during which a released dongle remains unavailable. |
| `scheduler` | Arbitration policy. Must be exactly `fifo` or `edf`. |

Example:

```bash
./codexion 5 800 20 10 10 3 5 edf
```

### Log format

Every state change uses the following format:

```text
timestamp_in_ms coder_id state
```

Possible messages are:

```text
X has taken a dongle
X is compiling
X is debugging
X is refactoring
X burned out
```

Each compile message is preceded by two dongle acquisition messages.

## Scheduling policies

Arbitration is performed independently by each dongle. Every coder request is
inserted into the priority queues of both adjacent dongles. A coder is granted
the pair only when it is at the head of both queues and both dongles are
available.

### FIFO

FIFO serves requests in arrival order. Each dongle assigns a monotonically
increasing rank when a request enters its queue. The smallest rank has the
highest priority.

### EDF

EDF prioritizes the earliest burnout deadline:

```text
deadline = last_compile_start + time_to_burnout
```

If two deadlines are equal, the lower coder ID is used as a deterministic
tie-breaker.

The scheduler grants both dongles atomically. A waiting coder never holds only
one dongle while waiting for the other.

### Priority queue implementation

Each dongle owns an array-based binary min-heap. The node at `queue[0]` is the
request with the highest scheduling priority. A node stores the coder ID, its
FIFO rank, and the absolute EDF deadline in microseconds.

`push_pq()` appends a node and moves it toward the root while it has a higher
priority than its parent. `pop_pq()` replaces the root with the last node and
moves that node down through the smaller-priority child. Insertion and removal
of the root therefore take `O(log n)` time. `remove_pq_at()` supports rollback
and cancellation of a request at an arbitrary index by rebuilding the heap in
`O(n)` time.

The heap uses a comparison function selected by the scheduler:

- FIFO compares the per-dongle insertion rank. The smaller rank wins.
- EDF compares the absolute burnout deadline. If deadlines are equal, the
  smaller coder ID wins.

A request is pushed into the heaps of both adjacent dongles while both dongle
mutexes are locked. The scheduler grants the request only when the same coder
is at the root of both heaps and both dongles are available. On allocation,
the two root nodes are removed and freed.

## Thread model

The program uses three kinds of threads:

- one worker thread for each coder;
- one monitor thread for burnout and completion detection;
- one scheduler timer thread, when there is more than one coder, for dongle
  cooldown expiration.

All created threads first enter a custom start barrier. Each thread reports its
arrival and waits on a condition variable. The main thread waits until every
expected worker has reached the barrier, resets the common simulation start
time and initial deadlines, and then broadcasts the start event.

The operating system still decides the exact CPU scheduling order after the
broadcast, but no thread can begin simulation work before every worker has
reached the barrier.

## Blocking cases handled

### Deadlock prevention and Coffman's conditions

The implementation prevents circular resource waiting in two complementary
ways:

- A coder receives both required dongles atomically or receives none. This
  removes the hold-and-wait condition from the normal multi-coder path.
- Whenever two dongle mutexes are needed, they are locked in ascending dongle
  ID order. A consistent global lock order prevents circular mutex waits.

Mutual exclusion remains necessary because dongles are non-shareable
resources. Preemption is not used: a compiling coder keeps both dongles until
the compile phase finishes. Deadlock is avoided by eliminating hold-and-wait
and circular wait rather than by removing mutual exclusion.

### Starvation prevention

Every dongle maintains its own priority queue. FIFO prevents later requests
from overtaking earlier requests on the same dongle. EDF applies the same
total ordering `(deadline, coder_id)` to every dongle, prioritizing the coder
closest to burnout and producing deterministic ties.

A request is removed only when both required dongles are granted. Therefore,
a request that is first on only one dongle cannot bypass a higher-priority
request on the other dongle.

Under feasible EDF parameters, the earliest pending deadline becomes eligible
as soon as its dongles are available. Running coders release their dongles
after a finite compile time, and cooldowns also have finite expiration times.

### Cooldown handling

Releasing a dongle changes its state to `COOLDOWN` and records an absolute
cooldown end time. The scheduler timer finds the nearest pending cooldown,
waits until that time, updates the dongle state, and runs the scheduler again.
This prevents all waiting coders from sleeping forever when time expiration is
the only state change.

### Precise burnout detection

A dedicated monitor thread checks each unfinished coder against:

```text
last_compile_start + time_to_burnout
```

The monitor is independent of worker progress and checks frequently enough to
emit the burnout message within the required tolerance. Once burnout is
detected, it changes the shared simulation state and wakes every waiting
thread so that shutdown can proceed.

### Log serialization

All output is protected by a dedicated logging mutex. This prevents messages
from different threads from interleaving on the same line. The two dongle
acquisition messages for one coder are printed while holding the same lock, so
another state message cannot appear between them.

### Startup and shutdown blocking

The custom start barrier prevents early-created threads from consuming part of
another thread's initial burnout interval. During shutdown, all relevant
condition variables are broadcast so that threads waiting for the scheduler,
the start event, or the single-coder dongle path can observe the terminal state
and exit.

## Thread synchronization mechanisms

### Simulation state mutex and condition variable

`state_mutex` protects the global simulation state and scheduler-visible coder
fields, including the simulation status, start barrier counters, compile
counts, deadlines, pending requests, and granted dongle pairs.

`state_cond` communicates state changes between coder threads, the monitor,
the scheduler timer, and the main thread. It is used for:

- the startup barrier;
- waiting for a dongle pair to be granted;
- notifying the scheduler after a release or deadline change;
- waking all threads when the simulation stops.

Every condition wait is enclosed in a loop that checks an explicit predicate.
For example, startup waits while `start_ready` is false, and scheduler waiting
continues while the coder has no dongles and the simulation is still running.
This makes spurious wakeups harmless.

`pthread_cond_wait()` atomically releases `state_mutex` while sleeping and
reacquires it before returning. Consequently, the thread that changes the
predicate can acquire the mutex without a lost-wakeup race.

### Dongle mutexes

Every dongle owns a mutex protecting:

- its `AVAILABLE`, `USING`, or `COOLDOWN` state;
- its cooldown deadline;
- its priority queue.

Operations that inspect or change two dongles lock both in ascending ID order.
The scheduler verifies availability and changes both states while holding
these locks, preventing two coders from being granted the same dongle.

The single-coder case has only one physical dongle. It uses the dongle's own
condition variable and a separate acquisition path to avoid locking the same
non-recursive mutex twice.

### Logging mutex

`log_mutex` serializes standard output independently of resource scheduling.
Before emitting an ordinary state message, the logger also verifies the shared
simulation state so that normal messages are suppressed after termination.

### Thread-safe communication example

When a coder releases its dongles:

1. Each dongle mutex is locked.
2. The state becomes `COOLDOWN` and its expiration time is stored.
3. The dongle mutex is unlocked.
4. `state_mutex` is locked and the coder's granted state is cleared.
5. `state_cond` is broadcast to notify scheduler waiters and the timer.

No thread relies on the notification alone. After waking, it reacquires the
associated mutex and checks the protected state again.

## Resources

- [JM Project - POSIX threads overview](https://linuxjm.sourceforge.io/html/LDP_man-pages/man7/pthreads.7.html)
- [JM Project - `pthread_create`](https://linuxjm.sourceforge.io/html/LDP_man-pages/man3/pthread_create.3.html)
- [JM Project - `pthread_join`](https://linuxjm.sourceforge.io/html/LDP_man-pages/man3/pthread_join.3.html)
- [JM Project - mutex operations](https://linuxjm.sourceforge.io/html/glibc-linuxthreads/man3/pthread_mutex_lock.3.html)
- [JM Project - condition variable operations](https://linuxjm.sourceforge.io/html/glibc-linuxthreads/man3/pthread_cond_wait.3.html)
- [Ｃで優先度付きキューを自作してみた](https://qiita.com/anonymously115/items/3b2948a26871615f7fea)
### AI Usage
- README Writing
- Testing
- Debugging