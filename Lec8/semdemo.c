/* A simple program to illustrate the use of semaphores.  Derived from:
 * https://www.beej.us/guide/bgipc/html/multi/semaphores.html
 *
 * Run in parallel in multiple processes to illustrate the blocking as
 * another process holds a lock.
 *
 * Use ipcs(1) to show that semaphores remain in the kernel until
 * explicitly removed.
 */
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#define MAX_RETRIES 10

#ifndef __DARWIN_UNIX03
union semun {
	int val;
	struct semid_ds *buf;
	ushort *array;
};
#endif

/*
* initsem() -- simplified version of W. Richard Stevens' UNIX Network
* Programming 2nd edition, volume 2, lockvsem.c, page 295;
*/
int
initsem(key_t key) {
	int i;
	union semun arg;
	struct semid_ds buf;
	struct sembuf sb;
	int semid;

	arg.buf = &buf;

	semid = semget(key, 1, IPC_CREAT | IPC_EXCL | 0666);
	if (semid >= 0) {       /* we got it first */
		sb.sem_num = 0; /* operate on the first semaphore in the set */
		sb.sem_op  = 1; /* "free" the semaphore */
		sb.sem_flg = 0; /* no flags are set */

		/* "Free" the semaphore.
		 * Note: since this semget, semop sequence is non-atomic,
		 * it poses a possible race condition whereby this
		 * process that created the semaphore might be suspended
		 * prior to freeing it.  In that case, the second process
		 * in the 'EEXIST' block below...
		 */
		if (semop(semid, &sb, 1) == -1) {
			int e = errno;
			if (semctl(semid, 0, IPC_RMID) < 0) {
				perror("semctl rm");
			}
			errno = e;
			return -1; /* error, check errno */
		}

	} else if (errno == EEXIST) { /* someone else got it first */
		int ready = 0;
		int e;

		semid = semget(key, 1, 0); /* get the id */
		if (semid < 0)
			return semid; /* error, check errno */

		/* ...would see sem_otime as '0', sleep for a second, and
		 * attempt again up to MAX_TRIES to give the
		 * semaphore-creating process above a chance to free it.
		 * Without this check, the race condition above could
		 * otherwise lead to a deadlock.
		 */
		for(i = 0; i < MAX_RETRIES && !ready; i++) {
			if ((e = semctl(semid, 0, IPC_STAT, arg)) < 0) {
				perror("semctl stat");
				return -1;
			}

			if (arg.buf->sem_otime != 0) {
				ready = 1;
			} else {
				sleep(1);
			}
		}

		/* If the other process didn't free the semaphore after
		 * MAX_TRIES seconds, give up.  Note: we set errno
		 * explicitly ourselves here. */
		if (!ready) {
			errno = ETIME;
			return -1;
		}
	} else {
		return semid; /* error, check errno */
	}

	return semid;
}

int
main(void) {
	key_t key;
	int semid;
	struct sembuf sb;

	sb.sem_num =  0;       /* we only operate on the first element in the semaphore set */
	sb.sem_op  = -1;       /* set to allocate resource */
	sb.sem_flg = SEM_UNDO; /* increment adjust on exit value by abs(sem_op) */

	/* Fun exercise: find out why we use 'J' here... */
	if ((key = ftok("semdemo.c", 'J')) == -1) {
		perror("ftok");
		exit(1);
	}

	/* What happens if you don't use ftok(3)?  Give it a try.
	key = IPC_PRIVATE;
	*/

	if ((semid = initsem(key)) == -1) {
		perror("initsem");
		exit(1);
	}

	printf("Press return to lock: ");
	(void)getchar();
	printf("Trying to lock...\n");

	/* This call will block if another process already has a lock. */
	if (semop(semid, &sb, 1) == -1) {
		perror("semop");
		exit(1);
	}

	printf("Locked.\n");
	printf("Press return to unlock: ");
	(void)getchar();

	/* Since we specified SEM_UNDO above, we don't stricly need to
	 * free the resource here; our process exiting would increment the
	 * semaphore value.  (Try it out: comment out this block and set
	 * sem_flg = 0 above.)
	 * However, just as with cleaning up file descriptors and other
	 * resources, it's a good idea to be explicit and free the
	 * resource on exit.*/
	sb.sem_op = 1;
	if (semop(semid, &sb, 1) == -1) {
		perror("semop");
		exit(1);
	}

	printf("Unlocked\n");

	return 0;
}
