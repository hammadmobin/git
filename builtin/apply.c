enum ws_error_action {
};

enum ws_ignore {
};

/*
 * We need to keep track of how symlinks in the preimage are
 * manipulated by the patches.  A patch to add a/b/c where a/b
 * is a symlink should not be allowed to affect the directory
 * the symlink points at, but if the same patch removes a/b,
 * it is perfectly fine, as the patch removes a/b to make room
 * to create a directory a/b so that a/b/c can be created.
 *
 * See also "struct string_list symlink_changes" in "struct
 * apply_state".
 */
#define SYMLINK_GOES_AWAY 01
#define SYMLINK_IN_RESULT 02
struct apply_state {
	const char *prefix;
	int prefix_length;

	/* These are lock_file related */
	struct lock_file *lock_file;
	int newfd;

	/* These control what gets looked at and modified */
	int apply; /* this is not a dry-run */
	int cached; /* apply to the index only */
	int check; /* preimage must match working tree, don't actually apply */
	int check_index; /* preimage must match the indexed version */
	int update_index; /* check_index && apply */

	/* These control cosmetic aspect of the output */
	int diffstat; /* just show a diffstat, and don't actually apply */
	int numstat; /* just show a numeric diffstat, and don't actually apply */
	int summary; /* just report creation, deletion, etc, and don't actually apply */

	/* These boolean parameters control how the apply is done */
	int allow_overlap;
	int apply_in_reverse;
	int apply_with_reject;
	int apply_verbosely;
	int no_add;
	int threeway;
	int unidiff_zero;
	int unsafe_paths;

	/* Other non boolean parameters */
	const char *fake_ancestor;
	const char *patch_input_file;
	int line_termination;
	struct strbuf root;
	int p_value;
	int p_value_known;
	unsigned int p_context;

	/* Exclude and include path parameters */
	struct string_list limit_by_name;
	int has_include;

	/* Various "current state" */
	int linenr; /* current line number */
	struct string_list symlink_changes; /* we have to track symlinks */
	/*
	 * For "diff-stat" like behaviour, we keep track of the biggest change
	 * we've seen, and the longest filename. That allows us to do simple
	 * scaling.
	 */
	int max_change;
	int max_len;
	/*
	 * Records filenames that have been touched, in order to handle
	 * the case where more than one patches touch the same file.
	 */
	struct string_list fn_table;

	/* These control whitespace errors */
	enum ws_error_action ws_error_action;
	enum ws_ignore ws_ignore_action;
	const char *whitespace_option;
	int whitespace_error;
	int squelch_whitespace_errors;
	int applied_after_fixing_ws;
};

static const char * const apply_usage[] = {
	N_("git apply [<options>] [<patch>...]"),
	NULL
};

static void parse_whitespace_option(struct apply_state *state, const char *option)
		state->ws_error_action = warn_on_ws_error;
		state->ws_error_action = warn_on_ws_error;
		state->ws_error_action = nowarn_ws_error;
		state->ws_error_action = die_on_ws_error;
		state->ws_error_action = die_on_ws_error;
		state->squelch_whitespace_errors = 0;
		state->ws_error_action = correct_ws_error;
static void parse_ignorewhitespace_option(struct apply_state *state,
					  const char *option)
		state->ws_ignore_action = ignore_ws_none;
		state->ws_ignore_action = ignore_ws_change;
static void set_default_whitespace_mode(struct apply_state *state)
	if (!state->whitespace_option && !apply_default_whitespace)
		state->ws_error_action = (state->apply ? warn_on_ws_error : nowarn_ws_error);
static int name_terminate(int c, int terminate)
static char *find_name_gnu(struct apply_state *state,
			   const char *line,
			   const char *def,
			   int p_value)
	if (state->root.len)
		strbuf_insert(&name, 0, state->root.buf, state->root.len);
static char *find_name_common(struct apply_state *state,
			      const char *line,
			      const char *def,
			      int p_value,
			      const char *end,
			      int terminate)
			if (name_terminate(c, terminate))
	if (state->root.len) {
		char *ret = xstrfmt("%s%.*s", state->root.buf, len, start);
static char *find_name(struct apply_state *state,
		       const char *line,
		       char *def,
		       int p_value,
		       int terminate)
		char *name = find_name_gnu(state, line, def, p_value);
	return find_name_common(state, line, def, p_value, NULL, terminate);
static char *find_name_traditional(struct apply_state *state,
				   const char *line,
				   char *def,
				   int p_value)
		char *name = find_name_gnu(state, line, def, p_value);
		return find_name_common(state, line, def, p_value, NULL, TERM_TAB);
	return find_name_common(state, line, def, p_value, line + len, 0);
static int guess_p_value(struct apply_state *state, const char *nameline)
	name = find_name_traditional(state, nameline, NULL, 0);
	else if (state->prefix) {
		if (!strncmp(name, state->prefix, state->prefix_length))
			val = count_slashes(state->prefix);
			if (!strncmp(cp, state->prefix, state->prefix_length))
				val = count_slashes(state->prefix) + 1;
static void parse_traditional_patch(struct apply_state *state,
				    const char *first,
				    const char *second,
				    struct patch *patch)
	if (!state->p_value_known) {
		p = guess_p_value(state, first);
		q = guess_p_value(state, second);
			state->p_value = p;
			state->p_value_known = 1;
		name = find_name_traditional(state, second, NULL, state->p_value);
		name = find_name_traditional(state, first, NULL, state->p_value);
		first_name = find_name_traditional(state, first, NULL, state->p_value);
		name = find_name_traditional(state, second, first_name, state->p_value);
		die(_("unable to find filename in patch at line %d"), state->linenr);
static int gitdiff_hdrend(struct apply_state *state,
			  const char *line,
			  struct patch *patch)
static void gitdiff_verify_name(struct apply_state *state,
				const char *line,
				int isnull,
				char **name,
				int side)
	if (!*name && !isnull) {
		*name = find_name(state, line, NULL, state->p_value, TERM_TAB);
		return;
	}
	if (*name) {
		int len = strlen(*name);
			    *name, state->linenr);
		another = find_name(state, line, NULL, state->p_value, TERM_TAB);
		if (!another || memcmp(another, *name, len + 1))
			    _("git apply: bad git-diff - inconsistent old filename on line %d"), state->linenr);
			die(_("git apply: bad git-diff - expected /dev/null on line %d"), state->linenr);
static int gitdiff_oldname(struct apply_state *state,
			   const char *line,
			   struct patch *patch)
	gitdiff_verify_name(state, line,
			    patch->is_new, &patch->old_name,
			    DIFF_OLD_NAME);
static int gitdiff_newname(struct apply_state *state,
			   const char *line,
			   struct patch *patch)
	gitdiff_verify_name(state, line,
			    patch->is_delete, &patch->new_name,
			    DIFF_NEW_NAME);
static int gitdiff_oldmode(struct apply_state *state,
			   const char *line,
			   struct patch *patch)
static int gitdiff_newmode(struct apply_state *state,
			   const char *line,
			   struct patch *patch)
static int gitdiff_delete(struct apply_state *state,
			  const char *line,
			  struct patch *patch)
	return gitdiff_oldmode(state, line, patch);
static int gitdiff_newfile(struct apply_state *state,
			   const char *line,
			   struct patch *patch)
	return gitdiff_newmode(state, line, patch);
static int gitdiff_copysrc(struct apply_state *state,
			   const char *line,
			   struct patch *patch)
	patch->old_name = find_name(state, line, NULL, state->p_value ? state->p_value - 1 : 0, 0);
static int gitdiff_copydst(struct apply_state *state,
			   const char *line,
			   struct patch *patch)
	patch->new_name = find_name(state, line, NULL, state->p_value ? state->p_value - 1 : 0, 0);
static int gitdiff_renamesrc(struct apply_state *state,
			     const char *line,
			     struct patch *patch)
	patch->old_name = find_name(state, line, NULL, state->p_value ? state->p_value - 1 : 0, 0);
static int gitdiff_renamedst(struct apply_state *state,
			     const char *line,
			     struct patch *patch)
	patch->new_name = find_name(state, line, NULL, state->p_value ? state->p_value - 1 : 0, 0);
static int gitdiff_similarity(struct apply_state *state,
			      const char *line,
			      struct patch *patch)
static int gitdiff_dissimilarity(struct apply_state *state,
				 const char *line,
				 struct patch *patch)
static int gitdiff_index(struct apply_state *state,
			 const char *line,
			 struct patch *patch)
static int gitdiff_unrecognized(struct apply_state *state,
				const char *line,
				struct patch *patch)
static const char *skip_tree_prefix(struct apply_state *state,
				    const char *line,
				    int llen)
	if (!state->p_value)
	nslash = state->p_value;
static char *git_header_name(struct apply_state *state,
			     const char *line,
			     int llen)
		cp = skip_tree_prefix(state, first.buf, first.len);
			cp = skip_tree_prefix(state, sp.buf, sp.len);
		cp = skip_tree_prefix(state, second, line + llen - second);
	name = skip_tree_prefix(state, line, llen);
			np = skip_tree_prefix(state, sp.buf, sp.len);
			second = skip_tree_prefix(state, name + len + 1,
static int parse_git_header(struct apply_state *state,
			    const char *line,
			    int len,
			    unsigned int size,
			    struct patch *patch)
	patch->def_name = git_header_name(state, line, len);
	if (patch->def_name && state->root.len) {
		char *s = xstrfmt("%s%s", state->root.buf, patch->def_name);
	state->linenr++;
	for (offset = len ; size > 0 ; offset += len, size -= len, line += len, state->linenr++) {
			int (*fn)(struct apply_state *, const char *, struct patch *);
			if (p->fn(state, line + oplen, patch) < 0)
static int find_header(struct apply_state *state,
		       const char *line,
		       unsigned long size,
		       int *hdrsize,
		       struct patch *patch)
	for (offset = 0; size > 0; offset += len, size -= len, line += len, state->linenr++) {
			    state->linenr, (int)len-1, line);
			int git_hdr_len = parse_git_header(state, line, len, size, patch);
					       state->p_value),
					    state->p_value, state->linenr);
				    "(line %d)", state->linenr);
		parse_traditional_patch(state, line, line+len, patch);
		state->linenr += 2;
static void record_ws_error(struct apply_state *state,
			    unsigned result,
			    const char *line,
			    int len,
			    int linenr)
	state->whitespace_error++;
	if (state->squelch_whitespace_errors &&
	    state->squelch_whitespace_errors < state->whitespace_error)
		state->patch_input_file, linenr, err, len, line);
static void check_whitespace(struct apply_state *state,
			     const char *line,
			     int len,
			     unsigned ws_rule)
	record_ws_error(state, result, line + 1, len - 2, state->linenr);
static int parse_fragment(struct apply_state *state,
			  const char *line,
			  unsigned long size,
			  struct patch *patch,
			  struct fragment *fragment)
	state->linenr++;
	     offset += len, size -= len, line += len, state->linenr++) {
			if (!state->apply_in_reverse &&
			    state->ws_error_action == correct_ws_error)
				check_whitespace(state, line, len, patch->ws_rule);
			if (state->apply_in_reverse &&
			    state->ws_error_action != nowarn_ws_error)
				check_whitespace(state, line, len, patch->ws_rule);
			if (!state->apply_in_reverse &&
			    state->ws_error_action != nowarn_ws_error)
				check_whitespace(state, line, len, patch->ws_rule);
static int parse_single_patch(struct apply_state *state,
			      const char *line,
			      unsigned long size,
			      struct patch *patch)
		fragment->linenr = state->linenr;
		len = parse_fragment(state, line, size, patch, fragment);
			die(_("corrupt patch at line %d"), state->linenr);
static struct fragment *parse_binary_hunk(struct apply_state *state,
					  char **buf_p,
	state->linenr++;
		state->linenr++;
	      state->linenr-1, llen-1, buffer);
static int parse_binary(struct apply_state *state,
			char *buffer,
			unsigned long size,
			struct patch *patch)
	forward = parse_binary_hunk(state, &buffer, &size, &status, &used);
		return error(_("unrecognized binary patch at line %d"), state->linenr-1);
	reverse = parse_binary_hunk(state, &buffer, &size, &status, &used_1);
static void prefix_one(struct apply_state *state, char **name)
	*name = xstrdup(prefix_filename(state->prefix, state->prefix_length, *name));
static void prefix_patch(struct apply_state *state, struct patch *p)
	if (!state->prefix || p->is_toplevel_relative)
	prefix_one(state, &p->new_name);
	prefix_one(state, &p->old_name);
static void add_name_limit(struct apply_state *state,
			   const char *name,
			   int exclude)
	it = string_list_append(&state->limit_by_name, name);
static int use_patch(struct apply_state *state, struct patch *p)
	if (0 < state->prefix_length) {
		if (pathlen <= state->prefix_length ||
		    memcmp(state->prefix, pathname, state->prefix_length))
	for (i = 0; i < state->limit_by_name.nr; i++) {
		struct string_list_item *it = &state->limit_by_name.items[i];
	return !state->has_include;
static int parse_chunk(struct apply_state *state, char *buffer, unsigned long size, struct patch *patch)
	int offset = find_header(state, buffer, size, &hdrsize, patch);
	prefix_patch(state, patch);
	if (!use_patch(state, patch))
	patchsize = parse_single_patch(state,
				       buffer + offset + hdrsize,
				       size - offset - hdrsize,
				       patch);
			state->linenr++;
			used = parse_binary(state, buffer + hd + llen,
					state->linenr++;
		if ((state->apply || state->check) &&
			die(_("patch with only garbage at line %d"), state->linenr);
static void show_stats(struct apply_state *state, struct patch *patch)
	max = state->max_len;
	max = max + state->max_change > 70 ? 70 - max : state->max_change;
	if (state->max_change > 0) {
		int total = ((add + del) * max + state->max_change / 2) / state->max_change;
		add = (add * max + state->max_change / 2) / state->max_change;
		size_t l_len = postimage->line[i].len;
			memmove(new, old, l_len);
			old += l_len;
			new += l_len;
		old += l_len;
		l_len = preimage->line[ctx].len;
		memcpy(new, fixed, l_len);
		new += l_len;
		fixed += l_len;
		postimage->line[i].len = l_len;
static int line_by_line_fuzzy_match(struct image *img,
				    struct image *preimage,
				    struct image *postimage,
				    unsigned long try,
				    int try_lno,
				    int preimage_limit)
{
	int i;
	size_t imgoff = 0;
	size_t preoff = 0;
	size_t postlen = postimage->len;
	size_t extra_chars;
	char *buf;
	char *preimage_eof;
	char *preimage_end;
	struct strbuf fixed;
	char *fixed_buf;
	size_t fixed_len;

	for (i = 0; i < preimage_limit; i++) {
		size_t prelen = preimage->line[i].len;
		size_t imglen = img->line[try_lno+i].len;

		if (!fuzzy_matchlines(img->buf + try + imgoff, imglen,
				      preimage->buf + preoff, prelen))
			return 0;
		if (preimage->line[i].flag & LINE_COMMON)
			postlen += imglen - prelen;
		imgoff += imglen;
		preoff += prelen;
	}

	/*
	 * Ok, the preimage matches with whitespace fuzz.
	 *
	 * imgoff now holds the true length of the target that
	 * matches the preimage before the end of the file.
	 *
	 * Count the number of characters in the preimage that fall
	 * beyond the end of the file and make sure that all of them
	 * are whitespace characters. (This can only happen if
	 * we are removing blank lines at the end of the file.)
	 */
	buf = preimage_eof = preimage->buf + preoff;
	for ( ; i < preimage->nr; i++)
		preoff += preimage->line[i].len;
	preimage_end = preimage->buf + preoff;
	for ( ; buf < preimage_end; buf++)
		if (!isspace(*buf))
			return 0;

	/*
	 * Update the preimage and the common postimage context
	 * lines to use the same whitespace as the target.
	 * If whitespace is missing in the target (i.e.
	 * if the preimage extends beyond the end of the file),
	 * use the whitespace from the preimage.
	 */
	extra_chars = preimage_end - preimage_eof;
	strbuf_init(&fixed, imgoff + extra_chars);
	strbuf_add(&fixed, img->buf + try, imgoff);
	strbuf_add(&fixed, preimage_eof, extra_chars);
	fixed_buf = strbuf_detach(&fixed, &fixed_len);
	update_pre_post_images(preimage, postimage,
			       fixed_buf, fixed_len, postlen);
	return 1;
}

static int match_fragment(struct apply_state *state,
			  struct image *img,
	} else if (state->ws_error_action == correct_ws_error &&
	if (state->ws_ignore_action == ignore_ws_change)
		return line_by_line_fuzzy_match(img, preimage, postimage,
						try, try_lno, preimage_limit);
	if (state->ws_error_action != correct_ws_error)
static int find_pos(struct apply_state *state,
		    struct image *img,
		if (match_fragment(state, img, preimage, postimage,
static void update_image(struct apply_state *state,
			 struct image *img,
	if (!state->allow_overlap)
static int apply_one_fragment(struct apply_state *state,
			      struct image *img, struct fragment *frag,
		if (state->apply_in_reverse) {
			if (first == '+' && state->no_add)
			    !state->whitespace_error ||
			    state->ws_error_action != correct_ws_error) {
				ws_fix_copy(&newlines, patch + 1, plen, ws_rule, &state->applied_after_fixing_ws);
			if (state->apply_verbosely)
			   (frag->oldpos == 1 && !state->unidiff_zero));
	match_end = !state->unidiff_zero && !trailing;
		applied_pos = find_pos(state, img, &preimage, &postimage, pos,
		if ((leading <= state->p_context) && (trailing <= state->p_context))
		    state->ws_error_action != nowarn_ws_error) {
			record_ws_error(state, WS_BLANK_AT_EOF, "+", 1,
			if (state->ws_error_action == correct_ws_error) {
			if (state->ws_error_action == die_on_ws_error)
				state->apply = 0;
		if (state->apply_verbosely && applied_pos != pos) {
			if (state->apply_in_reverse)
		update_image(state, img, applied_pos, &preimage, &postimage);
		if (state->apply_verbosely)
static int apply_binary_fragment(struct apply_state *state,
				 struct image *img,
				 struct patch *patch)
	if (state->apply_in_reverse) {
static int apply_binary(struct apply_state *state,
			struct image *img,
			struct patch *patch)
		if (apply_binary_fragment(state, img, patch))
static int apply_fragments(struct apply_state *state, struct image *img, struct patch *patch)
		return apply_binary(state, img, patch);
		if (apply_one_fragment(state, img, frag, inaccurate_eof, ws_rule, nth)) {
			if (!state->apply_with_reject)
static struct patch *in_fn_table(struct apply_state *state, const char *name)
	item = string_list_lookup(&state->fn_table, name);
static void add_to_fn_table(struct apply_state *state, struct patch *patch)
		item = string_list_insert(&state->fn_table, patch->new_name);
		item = string_list_insert(&state->fn_table, patch->old_name);
static void prepare_fn_table(struct apply_state *state, struct patch *patch)
			item = string_list_insert(&state->fn_table, patch->old_name);
static struct patch *previous_patch(struct apply_state *state,
				    struct patch *patch,
				    int *gone)
	previous = in_fn_table(state, patch->old_name);
static int load_patch_target(struct apply_state *state,
			     struct strbuf *buf,
	if (state->cached || state->check_index) {
			return error(_("failed to read %s"), name);
				return error(_("failed to read %s"), name);
static int load_preimage(struct apply_state *state,
			 struct image *image,
	previous = previous_patch(state, patch, &status);
		status = load_patch_target(state, &buf, ce, st,
			return error(_("failed to read %s"), patch->old_name);
static int load_current(struct apply_state *state,
			struct image *image,
			struct patch *patch)
	status = load_patch_target(state, &buf, ce, &st, name, mode);
static int try_threeway(struct apply_state *state,
			struct image *image,
			struct patch *patch,
			struct stat *st,
			const struct cache_entry *ce)
	if (apply_fragments(state, &tmp_image, patch) < 0) {
		if (load_current(state, &tmp_image, patch))
		if (load_preimage(state, &tmp_image, patch, st, ce))
static int apply_data(struct apply_state *state, struct patch *patch,
		      struct stat *st, const struct cache_entry *ce)
	if (load_preimage(state, &image, patch, st, ce) < 0)
	    apply_fragments(state, &image, patch) < 0) {
		if (!state->threeway || try_threeway(state, &image, patch, st, ce) < 0)
	add_to_fn_table(state, patch);
static int check_preimage(struct apply_state *state,
			  struct patch *patch,
			  struct cache_entry **ce,
			  struct stat *st)
	previous = previous_patch(state, patch, &status);
	} else if (!state->cached) {
	if (state->check_index && !previous) {
		if (!state->cached && verify_index_match(*ce, st))
		if (state->cached)
	if (!state->cached && !previous)
static int check_to_create(struct apply_state *state,
			   const char *new_name,
			   int ok_if_exists)
	if (state->check_index &&
	if (state->cached)
static uintptr_t register_symlink_changes(struct apply_state *state,
					  const char *path,
					  uintptr_t what)
	ent = string_list_lookup(&state->symlink_changes, path);
		ent = string_list_insert(&state->symlink_changes, path);
static uintptr_t check_symlink_changes(struct apply_state *state, const char *path)
	ent = string_list_lookup(&state->symlink_changes, path);
static void prepare_symlink_changes(struct apply_state *state, struct patch *patch)
			register_symlink_changes(state, patch->old_name, SYMLINK_GOES_AWAY);
			register_symlink_changes(state, patch->new_name, SYMLINK_IN_RESULT);
static int path_is_beyond_symlink_1(struct apply_state *state, struct strbuf *name)
		change = check_symlink_changes(state, name->buf);
		if (state->check_index) {
static int path_is_beyond_symlink(struct apply_state *state, const char *name_)
	ret = path_is_beyond_symlink_1(state, &name);
static int check_patch(struct apply_state *state, struct patch *patch)
	status = check_preimage(state, patch, &ce, &st);
	if ((tpatch = in_fn_table(state, new_name)) &&
		int err = check_to_create(state, new_name, ok_if_exists);
		if (err && state->threeway) {
	if (!state->unsafe_paths)
	if (!patch->is_delete && path_is_beyond_symlink(state, patch->new_name))
	if (apply_data(state, patch, &st, ce) < 0)
static int check_patch_list(struct apply_state *state, struct patch *patch)
	prepare_symlink_changes(state, patch);
	prepare_fn_table(state, patch);
		if (state->apply_verbosely)
		err |= check_patch(state, patch);
static void stat_patch_list(struct apply_state *state, struct patch *patch)
		show_stats(state, patch);
static void numstat_patch_list(struct apply_state *state,
			       struct patch *patch)
		write_name_quoted(name, stdout, state->line_termination);
static void patch_stats(struct apply_state *state, struct patch *patch)
	if (lines > state->max_change)
		state->max_change = lines;
		if (len > state->max_len)
			state->max_len = len;
		if (len > state->max_len)
			state->max_len = len;
static void remove_file(struct apply_state *state, struct patch *patch, int rmdir_empty)
	if (state->update_index) {
	if (!state->cached) {
static void add_index_file(struct apply_state *state,
			   const char *path,
			   unsigned mode,
			   void *buf,
			   unsigned long size)
	if (!state->update_index)
		if (!state->cached) {
static void create_one_file(struct apply_state *state,
			    char *path,
			    unsigned mode,
			    const char *buf,
			    unsigned long size)
	if (state->cached)
static void add_conflicted_stages_file(struct apply_state *state,
				       struct patch *patch)
	if (!state->update_index)
static void create_file(struct apply_state *state, struct patch *patch)
	create_one_file(state, path, mode, buf, size);
		add_conflicted_stages_file(state, patch);
		add_index_file(state, path, mode, buf, size);
static void write_out_one_result(struct apply_state *state,
				 struct patch *patch,
				 int phase)
			remove_file(state, patch, 1);
			create_file(state, patch);
		remove_file(state, patch, patch->is_rename);
		create_file(state, patch);
static int write_out_one_reject(struct apply_state *state, struct patch *patch)
		if (state->apply_verbosely)
static int write_out_results(struct apply_state *state, struct patch *list)
				write_out_one_result(state, l, phase);
					if (write_out_one_reject(state, l))
static int apply_patch(struct apply_state *state,
		       int fd,
		       const char *filename,
		       int options)
	state->patch_input_file = filename;
		nr = parse_chunk(state, buf.buf + offset, buf.len - offset, patch);
		if (state->apply_in_reverse)
		if (use_patch(state, patch)) {
			patch_stats(state, patch);
			if (state->apply_verbosely)
	if (state->whitespace_error && (state->ws_error_action == die_on_ws_error))
		state->apply = 0;
	state->update_index = state->check_index && state->apply;
	if (state->update_index && state->newfd < 0)
		state->newfd = hold_locked_index(state->lock_file, 1);
	if (state->check_index) {
	if ((state->check || state->apply) &&
	    check_patch_list(state, list) < 0 &&
	    !state->apply_with_reject)
	if (state->apply && write_out_results(state, list)) {
		if (state->apply_with_reject)
	if (state->fake_ancestor)
		build_fake_ancestor(list, state->fake_ancestor);
	if (state->diffstat)
		stat_patch_list(state, list);
	if (state->numstat)
		numstat_patch_list(state, list);
	if (state->summary)
	string_list_clear(&state->fn_table, 0);
	struct apply_state *state = opt->value;
	add_name_limit(state, arg, 1);
	struct apply_state *state = opt->value;
	add_name_limit(state, arg, 0);
	state->has_include = 1;
			  const char *arg,
			  int unset)
	struct apply_state *state = opt->value;
	state->p_value = atoi(arg);
	state->p_value_known = 1;
				     const char *arg, int unset)
	struct apply_state *state = opt->value;
		state->ws_ignore_action = ignore_ws_none;
		state->ws_ignore_action = ignore_ws_change;
	struct apply_state *state = opt->value;
	state->whitespace_option = arg;
	parse_whitespace_option(state, arg);
	struct apply_state *state = opt->value;
	strbuf_reset(&state->root);
	strbuf_addstr(&state->root, arg);
	strbuf_complete(&state->root, '/');
static void init_apply_state(struct apply_state *state,
			     const char *prefix,
			     struct lock_file *lock_file)
{
	memset(state, 0, sizeof(*state));
	state->prefix = prefix;
	state->prefix_length = state->prefix ? strlen(state->prefix) : 0;
	state->lock_file = lock_file;
	state->newfd = -1;
	state->apply = 1;
	state->line_termination = '\n';
	state->p_value = 1;
	state->p_context = UINT_MAX;
	state->squelch_whitespace_errors = 5;
	state->ws_error_action = warn_on_ws_error;
	state->ws_ignore_action = ignore_ws_none;
	state->linenr = 1;
	string_list_init(&state->fn_table, 0);
	string_list_init(&state->limit_by_name, 0);
	string_list_init(&state->symlink_changes, 0);
	strbuf_init(&state->root, 0);

	git_apply_config();
	if (apply_default_whitespace)
		parse_whitespace_option(state, apply_default_whitespace);
	if (apply_default_ignorewhitespace)
		parse_ignorewhitespace_option(state, apply_default_ignorewhitespace);
}

static void clear_apply_state(struct apply_state *state)
{
	string_list_clear(&state->limit_by_name, 0);
	string_list_clear(&state->symlink_changes, 0);
	strbuf_release(&state->root);

	/* &state->fn_table is cleared at the end of apply_patch() */
}

static void check_apply_state(struct apply_state *state, int force_apply)
{
	int is_not_gitdir = !startup_info->have_repository;

	if (state->apply_with_reject && state->threeway)
		die("--reject and --3way cannot be used together.");
	if (state->cached && state->threeway)
		die("--cached and --3way cannot be used together.");
	if (state->threeway) {
		if (is_not_gitdir)
			die(_("--3way outside a repository"));
		state->check_index = 1;
	}
	if (state->apply_with_reject)
		state->apply = state->apply_verbosely = 1;
	if (!force_apply && (state->diffstat || state->numstat || state->summary || state->check || state->fake_ancestor))
		state->apply = 0;
	if (state->check_index && is_not_gitdir)
		die(_("--index outside a repository"));
	if (state->cached) {
		if (is_not_gitdir)
			die(_("--cached outside a repository"));
		state->check_index = 1;
	}
	if (state->check_index)
		state->unsafe_paths = 0;
	if (!state->lock_file)
		die("BUG: state->lock_file should not be NULL");
}

static int apply_all_patches(struct apply_state *state,
			     int argc,
			     const char **argv,
			     int options)
	int read_stdin = 1;

	for (i = 0; i < argc; i++) {
		const char *arg = argv[i];
		int fd;

		if (!strcmp(arg, "-")) {
			errs |= apply_patch(state, 0, "<stdin>", options);
			read_stdin = 0;
			continue;
		} else if (0 < state->prefix_length)
			arg = prefix_filename(state->prefix,
					      state->prefix_length,
					      arg);

		fd = open(arg, O_RDONLY);
		if (fd < 0)
			die_errno(_("can't open patch '%s'"), arg);
		read_stdin = 0;
		set_default_whitespace_mode(state);
		errs |= apply_patch(state, fd, arg, options);
		close(fd);
	}
	set_default_whitespace_mode(state);
	if (read_stdin)
		errs |= apply_patch(state, 0, "<stdin>", options);

	if (state->whitespace_error) {
		if (state->squelch_whitespace_errors &&
		    state->squelch_whitespace_errors < state->whitespace_error) {
			int squelched =
				state->whitespace_error - state->squelch_whitespace_errors;
			warning(Q_("squelched %d whitespace error",
				   "squelched %d whitespace errors",
				   squelched),
				squelched);
		}
		if (state->ws_error_action == die_on_ws_error)
			die(Q_("%d line adds whitespace errors.",
			       "%d lines add whitespace errors.",
			       state->whitespace_error),
			    state->whitespace_error);
		if (state->applied_after_fixing_ws && state->apply)
			warning("%d line%s applied after"
				" fixing whitespace errors.",
				state->applied_after_fixing_ws,
				state->applied_after_fixing_ws == 1 ? "" : "s");
		else if (state->whitespace_error)
			warning(Q_("%d line adds whitespace errors.",
				   "%d lines add whitespace errors.",
				   state->whitespace_error),
				state->whitespace_error);
	}

	if (state->update_index) {
		if (write_locked_index(&the_index, state->lock_file, COMMIT_LOCK))
			die(_("Unable to write new index file"));
		state->newfd = -1;
	}

	return !!errs;
}
int cmd_apply(int argc, const char **argv, const char *prefix)
{
	int force_apply = 0;
	int options = 0;
	int ret;
	struct apply_state state;
		{ OPTION_CALLBACK, 0, "exclude", &state, N_("path"),
		{ OPTION_CALLBACK, 0, "include", &state, N_("path"),
		{ OPTION_CALLBACK, 'p', NULL, &state, N_("num"),
		OPT_BOOL(0, "no-add", &state.no_add,
		OPT_BOOL(0, "stat", &state.diffstat,
		OPT_BOOL(0, "numstat", &state.numstat,
		OPT_BOOL(0, "summary", &state.summary,
		OPT_BOOL(0, "check", &state.check,
		OPT_BOOL(0, "index", &state.check_index,
		OPT_BOOL(0, "cached", &state.cached,
		OPT_BOOL(0, "unsafe-paths", &state.unsafe_paths,
		OPT_BOOL('3', "3way", &state.threeway,
		OPT_FILENAME(0, "build-fake-ancestor", &state.fake_ancestor,
		OPT_SET_INT('z', NULL, &state.line_termination,
		OPT_INTEGER('C', NULL, &state.p_context,
		{ OPTION_CALLBACK, 0, "whitespace", &state, N_("action"),
		{ OPTION_CALLBACK, 0, "ignore-space-change", &state, NULL,
		{ OPTION_CALLBACK, 0, "ignore-whitespace", &state, NULL,
		OPT_BOOL('R', "reverse", &state.apply_in_reverse,
		OPT_BOOL(0, "unidiff-zero", &state.unidiff_zero,
		OPT_BOOL(0, "reject", &state.apply_with_reject,
		OPT_BOOL(0, "allow-overlap", &state.allow_overlap,
		OPT__VERBOSE(&state.apply_verbosely, N_("be verbose")),
		{ OPTION_CALLBACK, 0, "directory", &state, N_("root"),
	init_apply_state(&state, prefix, &lock_file);
	argc = parse_options(argc, argv, state.prefix, builtin_apply_options,
	check_apply_state(&state, force_apply);
	ret = apply_all_patches(&state, argc, argv, options);
	clear_apply_state(&state);
	return ret;