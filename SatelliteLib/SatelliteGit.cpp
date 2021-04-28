#include "SatelliteGit.h"
#include <string>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stack>
#include <string.h>

#include <GitProxy.h>
#include <Utility.h>
#include <CheckoutInfo.h>

#include <nlohmann/json.hpp>

namespace Sat
{

void SatelliteGit::load()
{
    GitProxy::git_libgit2_init();
}

void SatelliteGit::unload()
{
    GitProxy::git_libgit2_shutdown();
}

CheckoutInfo SatelliteGit::checkout_info(char const* p_targetPath)
{    
    GitProxy::GitBuf buf{};
    GitProxy::GitRepo* repo{};
    int discoverResult{};
    int repoOpenResult{};
    char const * workdirCharPtr{};
    std::unique_ptr<char const[]> workdir{}; //Use unique_ptr because it is nullable unlike std::string

    discoverResult = GitProxy::git_repository_discover(&buf, p_targetPath, false, nullptr);
    if(discoverResult != 0 || buf.m_ptr == nullptr)
    {
        goto cleanup;
    }

    repoOpenResult = GitProxy::git_repository_open(&repo, buf.m_ptr);
    if (repoOpenResult != 0)
    {
        goto cleanup;
    }


    workdirCharPtr = GitProxy::git_repository_workdir(repo);
    if(workdirCharPtr)
    {
        size_t length = std::strlen(workdirCharPtr)+1;
        std::unique_ptr<char []> copyBuffer = std::make_unique<char []>(length); // +1 for the null terminator       
        strcpy_s(copyBuffer.get(), length*sizeof(char), workdirCharPtr);
        workdir = std::move(copyBuffer);
    }

cleanup:

    if (repoOpenResult == 0)
    {
        GitProxy::git_repository_free(repo);
    }

    if (discoverResult == 0)
    {
        GitProxy::git_buf_free(&buf);
    }

    return CheckoutInfo(discoverResult == 0, std::move(workdir));
}

}