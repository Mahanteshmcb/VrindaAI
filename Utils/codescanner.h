#ifndef CODESCANNER_H
#define CODESCANNER_H

#include <QString>

class CodeScanner
{
public:
    CodeScanner();
    QString summarizeProject(const QString &projectPath);
};

#endif // CODESCANNER_H
