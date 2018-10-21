#pragma once


typedef CRITICAL_SECTION Critical;


class Thread {
protected:
	HANDLE		hRun;		// Main thread will raise this event (Wake) when there's work to do (or we should stop).
	HANDLE		hStop;		// This thread will raise this event when it has stopped working (forever).
	Critical	*pCS;
	bool		Abort;

	static void ThreadFunc(void *pThis);

public:
	Thread(Critical *pCS) : pCS(pCS), Abort(false) {
		hRun = CreateEvent(NULL, FALSE, FALSE, NULL);
		hStop = CreateEvent(NULL, TRUE, FALSE, NULL);

		_beginthread(ThreadFunc, 0, this);
	}

	~Thread() {
		Abort = true;
		SetEvent(hRun);
		WaitForSingleObject(hStop, INFINITE);
		CloseHandle(hStop);
		CloseHandle(hRun);
	}

	void Wake() {
		SetEvent(hRun);
	}
};
