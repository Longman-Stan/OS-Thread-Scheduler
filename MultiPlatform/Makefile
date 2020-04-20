CFLAGS = /nologo /W3 /EHsc
CC = cl
LINK = link

build: libscheduler.dll

libscheduler.dll: queue.obj hash.obj so_scheduler.obj so_thread.obj
	$(LINK) /nologo /dll /out:$@ /implib:libscheduler.lib $**

hash.obj: queue.obj hash.c
	$(CC) $(CFLAGS) /Fo$@ /c hash.c
	
queue.obj: so_thread.obj queue.c
	$(CC) $(CFLAGS) /Fo$@ /c queue.c

so_thread.obj: so_scheduler.obj so_thread.c
	$(CC) $(CFLAGS) /Fo$@ /c so_thread.c

so_scheduler.obj: so_scheduler.c
	$(CC) $(CFLAGS) /Fo$@ /c so_scheduler.c

clean:
	del /Q hash.obj queue.obj so_thread.obj so_scheduler.obj 2>NUL
	del /Q libscheduler.dll libscheduler.lib libscheduler.exp 2>NUL
