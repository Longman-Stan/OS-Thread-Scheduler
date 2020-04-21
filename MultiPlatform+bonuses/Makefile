CFLAGS = /nologo /W3 /EHsc
CC = cl
LINK = link

build: libscheduler.dll

bonus_tests: bonus_test_exec bonus_test_wait config_params

bonus_test_exec: bonus_test_exec.c
	$(CC) /W3 /Fobonus_test_exec.exe bonus_test_exec.c /LIBPATH:. libscheduler.lib

bonus_test_wait: bonus_test_wait.c
	$(CC) /W3 /Fobonus_test_wait.exe bonus_test_wait.c /LIBPATH:. libscheduler.lib

config_params: config_params.c
	$(CC) /W3 /Foconfig_params.exe config_params.c

libscheduler.dll: queue.obj hash.obj so_scheduler.obj so_thread.obj so_sched_initializer.obj so_logger.obj
	$(LINK) /nologo /dll /out:$@ /implib:libscheduler.lib $**

hash.obj: queue.obj hash.c
	$(CC) $(CFLAGS) /Fo$@ /c hash.c
	
queue.obj: so_thread.obj queue.c
	$(CC) $(CFLAGS) /Fo$@ /c queue.c

so_thread.obj: so_scheduler.obj so_thread.c
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
	rm bonus_test_exec bonus_test_wait config_params