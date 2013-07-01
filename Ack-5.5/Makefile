TARGETS = dis88 lib ld ar

all clean:
	for dir in $(TARGETS); do ($(MAKE) $@ -C $$dir || exit 1); done
