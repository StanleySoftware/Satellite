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

Error SatelliteGit::checkout_info(char const* p_targetPath, CheckoutInfo& p_out_checkoutInfo)
{    
    GitProxy::GitBuf buf{};
    GitProxy::GitRepo* repo{};
    int discoverResult = -1;
    int repoOpenResult = -1;
    char const * workdirCharPtr{};
    CStringWrapper workdir{}; //Use CStringWrapper because it is nullable unlike std::string

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
        CStringWrapper copyBuffer(length);
        strcpy_s(copyBuffer.get(), length*sizeof(char), workdirCharPtr);
        workdir = std::move(copyBuffer);
    }

    p_out_checkoutInfo.m_isCheckout = discoverResult == 0;
    p_out_checkoutInfo.m_checkoutRoot = std::move(workdir);

cleanup:

    if (repoOpenResult == 0)
    {
        GitProxy::git_repository_free(repo);
    }

    if (discoverResult == 0)
    {
        GitProxy::git_buf_free(&buf);
    }

    return Error{};
}

}