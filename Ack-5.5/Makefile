TARGETS = dis88 lib ld ar cv

all clean:
	for dir in $(TARGETS); do ($(MAKE) $@ -C $$dir || exit 1); done
