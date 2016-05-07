



extern
bool file2info(
	       const char *const,
	       char **const,
	       struct proc_info_t *const);


extern
bool info2file(
	       const char *const,
	       const char *const,
	       const struct proc_info_t *const);


extern
bool instruction2file(
		      const char *,
		      const char*,
		      const uint8_t *const,
		      const size_t);
