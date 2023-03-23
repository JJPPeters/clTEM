//
// Created by jonat on 23/03/2023.
//

#include "getversion.h"

extern "C"
{
extern const char* GIT_TAG;
extern const char* GIT_REV;
extern const char* GIT_BRANCH;
}

const char* git_version(void)
{
    return GIT_TAG;
}

const char* git_revision(void)
{
    return GIT_REV;
}

const char* git_branch(void)
{
    return GIT_BRANCH;
}

std::string make_version_string() {
    std::string out = "clTEM - ";
    std::string ver = git_version();
    std::string hsh = git_revision();
    std::string brn = git_branch();
    if (!ver.empty())
        return out + ver + " - " + hsh;
    else
        return out + brn + " - " + hsh;
}