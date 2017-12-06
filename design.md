# Rate Monotonic Scheduler

As a brief overview, the schedule() function first creates 4 worker
threads with decreasing priority and creates a semaphore for each,
initialized to zero. Every time their job is scheduled, their Semaphore
gets incremented (IE signal()) and subsequently the workers are awaken
from their wait().

Unfortunately, despite all my code making sense to me, I can't seem to
properly set priority. Because such, worker3 seems to be favored in
runtime, even over the scheduler, so no overruns ever occur. This is
obviously a bit of a bummer, because I believe all other portions of
the code to be correct.
