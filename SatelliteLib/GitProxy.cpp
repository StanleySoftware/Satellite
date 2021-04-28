#include "GitProxy.h"
#include <stdexcept>
#include <DllLoad.h>
#include <Utility.h>

namespace Sat
{

namespace GitProxy
{

static constexpr char const * s_libgitFilename = "git2-6777db8.dll";
static LibLoad::LibObject s_libgit(s_libgitFilename);

using GitLibGitInitFn = int (*) ();
static_assert(std::is_same_v<decltype(&git_libgit2_init), GitLibGitInitFn>, "git_libgit2_init should be of type GitLibGitInitFn");

using GitLibGitShutdownFn = int (*) ();
static_assert(std::is_same_v<decltype(&git_libgit2_shutdown), GitLibGitShutdownFn>, "git_libgit2_shutdown should be of type GitLibGitShutdownFn");

using GitBufFreeFn = void (*) (GitBuf*);
static_assert(std::is_same_v<decltype(&git_buf_free), GitBufFreeFn>, "git_buf_dispose should be of type GitBufDisposeFn");

using GitRepositoryDiscoverFn = int (*) (GitBuf*, char const*, int, char const*);
static_assert(std::is_same_v<decltype(&git_repository_discover), GitRepositoryDiscoverFn>, "git_repository_discover should be of type GitRepositoryDiscoverFn");

using GitRepositoryOpenFn = int (*) (GitRepo**, char const *);
static_assert(std::is_same_v<decltype(&git_repository_open), GitRepositoryOpenFn>, "git_repository_open should be of type GitRepositoryOpenFn");

using GitRepositoryFreeFn = void (*) (GitRepo*);
static_assert(std::is_same_v<decltype(&git_repository_free), GitRepositoryFreeFn>, "git_repository_free should be of type GitRepositoryFreeFn");

using GitRepositoryWorkDirFn = char const * (*) (GitRepo*);
static_assert(std::is_same_v<decltype(&git_repository_workdir), GitRepositoryWorkDirFn>, "git_repository_workdir should be of type GitRepositoryWorkDirFn");

template<typename FuncPtrType, typename ...Args>
auto invoke_lib_func(char const * const p_func_name, Args... p_args) -> decltype(std::declval<FuncPtrType>()(std::declval<Args>()...))
{
	FuncPtrType func_to_invoke = s_libgit.get_proc<FuncPtrType>(p_func_name);
	if(!func_to_invoke)
	{
		std::string err_str = str_format("Unable to invoke function '%s' from library '%s'.", p_func_name, s_libgit.get_lib_name());
		throw std::runtime_error(err_str);
	}
	
	return func_to_invoke(std::forward<Args>(p_args)...);
}

int git_libgit2_init()
{
	s_libgit.load();
	constexpr char const * func_name = "git_libgit2_init";
	return invoke_lib_func<GitLibGitInitFn>(func_name);
}

int git_libgit2_shutdown()
{
	constexpr char const * func_name = "git_libgit2_shutdown";
	int count = invoke_lib_func<GitLibGitShutdownFn>(func_name);
	s_libgit.unload();
	return count;
}

void git_buf_free(GitBuf* p_buf)
{
	constexpr char const * func_name = "git_buf_free";
	return invoke_lib_func<GitBufFreeFn>(func_name, p_buf);
}

int git_repository_discover(GitBuf* p_out_buf, char const* p_start_path, int p_across_fs, char const* p_ceiling_dirs)
{
	constexpr char const * func_name = "git_repository_discover";
	return invoke_lib_func<GitRepositoryDiscoverFn>(func_name, p_out_buf, p_start_path, p_across_fs, p_ceiling_dirs);
}

int git_repository_open(GitRepo** p_out_repo, char const* p_path)
{
	constexpr char const * func_name = "git_repository_open";
	return invoke_lib_func<GitRepositoryOpenFn>(func_name, p_out_repo, p_path);
}

void git_repository_free(GitRepo* p_repo)
{
	constexpr char const * func_name = "git_repository_free";
	return invoke_lib_func<GitRepositoryFreeFn>(func_name, p_repo);
}

char const* git_repository_workdir(GitRepo* p_repo)
{
	constexpr char const * func_name = "git_repository_workdir";
	return invoke_lib_func<GitRepositoryWorkDirFn>(func_name, p_repo);
}

}

}