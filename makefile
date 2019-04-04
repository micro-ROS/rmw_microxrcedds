.PHONY: clean

.DEFAULT: cache
	gitlab-runner exec docker --docker-volumes `pwd`/cache:/cache $@_local

cache:
	mkdir $@

clean:
	rm -rf cache
