#define _GNU_SOURCE

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "common.h"
#include "exedir.h"

#define TEMPNAMESZ 1024

static char tempdir[TEMPNAMESZ];

static
void _cd_exedir()
{
  // Attempt to use /tmp instead of the HOME directory
  // This ensures all temporary files are on the local machine
  // (not NFS mounted). Thus we make an effort to ensure
  // different users don't interfere with one another.
#if 0
  const char *const home = getenv("HOME");

  if (!home) {
    fprintf(stderr, "HOME not set");
    exit(EXIT_FAILURE);
  }
#endif
  const char *const home = "/tmp";

  REQUIRE (chdir(home) == 0);

  const int retcode = snprintf(tempdir, TEMPNAMESZ, ".rappel%d", getuid());

  if(retcode < 0 || retcode >= TEMPNAMESZ){
    fprintf(stderr, "snprintf in  _cd_exedir failed\n");
    exit(EXIT_FAILURE);
  }
    
  
  if (mkdir(tempdir, 0755) == -1) {
    if (errno != EEXIST) {
      perror("mkdir");
      exit(EXIT_FAILURE);
    }
  }
  
  REQUIRE (chdir(tempdir) == 0);
}

static const
int _reopen_ro(
		const int h,
		const char *const path)
{
	REQUIRE (close(h) == 0);

	const int ro_h = open(path, O_RDONLY | O_CLOEXEC);

	if (ro_h < 0) {
		perror("open");
		exit(EXIT_FAILURE);
	}

	return ro_h;
}

void clean_exedir()
{
	char initial_cwd[PATH_MAX];

	 REQUIRE (getcwd(initial_cwd, PATH_MAX) != NULL);

	_cd_exedir();

	DIR *exedir = opendir(".");

	if (!exedir) {
		perror("opendir");
		exit(EXIT_FAILURE);
	}
	
	struct dirent *f;
	while ((f = readdir(exedir))) {
		if (!strcmp(f->d_name, ".") || !strcmp(f->d_name, ".."))
			continue;
		
		if (unlink(f->d_name) == -1)
			fprintf(stderr, "Cannot unlink: %s", f->d_name);
	}

	REQUIRE (closedir(exedir) == 0);

	REQUIRE (chdir(initial_cwd) == 0);
}

const
int write_exe(
		const uint8_t *data,
		const size_t data_sz,
		const char *name)
{
	if (name)
		return write_named_file(data, data_sz, name);
	else
		return write_tmp_file(data, data_sz);
}

const
int write_named_file(
		const uint8_t *data,
		const size_t data_sz,
		const char *name)
{
	const int h = open(name, O_WRONLY | O_CREAT, 
			S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);

	if (h < 0) {
		perror("open");
		exit(EXIT_FAILURE);
	}

	write_data(h, data, data_sz);

	return _reopen_ro(h, name);
}

const
int write_tmp_file(
		const uint8_t *data,
		const size_t data_sz)
{
	char path[PATH_MAX], initial_cwd[PATH_MAX];

	REQUIRE (getcwd(initial_cwd, PATH_MAX) != NULL);

	_cd_exedir();

	snprintf(path, sizeof(path), "rappel-exec.XXXXXX");

	const int h = mkstemp(path);

	if (h < 0) {
		perror("mkstemp");
		exit(EXIT_FAILURE);
	}

	write_data(h, data, data_sz);

	REQUIRE (fchmod(h, S_IXUSR | S_IRUSR | S_IWUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) == 0);

	const int h_ro = _reopen_ro(h, path);

	REQUIRE (chdir(initial_cwd) == 0);

	return h_ro;
}
