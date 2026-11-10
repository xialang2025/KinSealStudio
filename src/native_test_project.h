#ifndef NATIVE_TEST_PROJECT_H
#define NATIVE_TEST_PROJECT_H

#include "project_model.h"

bool exportNativeTestProject(const HmiProject &project, const QString &path,
                             QString *error);
bool importNativeTestProject(const QString &path, HmiProject *project,
                            QString *error); //20261108-9 add

#endif
