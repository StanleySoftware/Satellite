#pragma once

#include <type_traits>
#include <DllLoad.h>

namespace Sat
{

namespace GitProxy
{

//Forward Declare

struct GitBuf;
struct GitRepo;

//Declare

int git_libgit2_init();
int git_libgit2_shutdown();

void git_buf_free(GitBuf* p_buf);

int git_repository_discover(GitBuf* p_out_buf, char const* p_start_path, int p_across_fs, char const* p_ceiling_dirs);
int git_repository_open(GitRepo** p_out_repo, char const* p_path);
void git_repository_free(GitRepo* p_repo);
char const * git_repository_workdir(GitRepo* p_repo);

//Define

struct GitBuf
{
    char * m_ptr;
    unsigned int* m_asize;
    unsigned int* m_size;
};

struct GitRepo
{};

}

}