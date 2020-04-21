CFLAGS = /nologo /W3 /EHsc /D_CRT_SECURE_NO_DEPRECATE
CC = cl
LINK = link

build: libscheduler.dll

bonus_tests:  libscheduler2.dll bonus_test_exec bonus_test_wait config_params

bonus_test_exec: bonus_test_exec.c
	$(CC) /W3 /Fo$@ bonus_test_exec.c libscheduler2.lib

bonus_test_wait: bonus_test_wait.c
	$(CC) /W3 /Fo$@ bonus_test_wait.c libscheduler2.lib

config_params: config_params.c
	$(CC) /W3 /D_CRT_SECURE_NO_DEPRECATE config_params.c so_sched_initializer.c

libscheduler2.dll: queue.obj hash.obj so_scheduler2.obj so_thread.obj so_sched_initializer.obj so_logger.obj
	$(LINK) /nologo /dll /out:$@ /implib:libscheduler2.lib $**

so_scheduler2.obj: so_scheduler.c
	$(CC) $(CFLAGS) /Fo$@ /c so_scheduler.c

libscheduler.dll: queue.obj hash.obj so_scheduler.obj so_thread.obj so_sched_initializer.obj so_logger.obj
	$(LINK) /nologo /dll /out:$@ /implib:libscheduler.lib $**

hash.obj: hash.c
	$(CC) $(CFLAGS) /Fo$@ /c hash.c
	
queue.obj: queue.c
	$(CC) $(CFLAGS) /Fo$@ /c queue.c

so_thread.obj: so_thread.c
	$(CC) $(CFLAGS) /Fo$@ /c so_thread.c

so_scheduler.obj: so_scheduler.c
	$(CC) $(CFLAGS) /Fo$@ /c so_scheduler.c

so_logger.obj: so_logger.c
	$(CC) $(CFLAGS) /Fo$@ /c so_logger.c

so_sched_initializer.obj: so_sched_initializer.c
	$(CC) $(CFLAGS) /Fo$@ /c so_sched_initializer.c

clean:
	del /Q hash.obj queue.obj so_thread.obj so_scheduler.obj so_logger.obj so_sched_initializer.obj 2>NUL
	del /Q libscheduler.dll libscheduler.lib libscheduler.exp 2>NUL

clean_bonus:
	del /Q bonus_test_exec.obj bonus_test_wait.obj config_params.obj 2>NUL
	del /Q bonus_test_exec.exe bonus_test_wait.exe config_params.exe 2>NUL
	del /Q hash.obj queue.obj so_thread.obj so_scheduler2.obj so_logger.obj so_sched_initializer.obj 2>NUL
	del /Q libscheduler2.dll libscheduler2.lib libscheduler2.exp 2>NUL