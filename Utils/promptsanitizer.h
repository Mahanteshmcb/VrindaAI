#ifndef PROMPTSANITIZER_H
#define PROMPTSANITIZER_H

#include <QString>

class PromptSanitizer
{
public:
    // This is a static function, callable directly via PromptSanitizer::sanitize()
    static QString sanitizePromptForCli(const QString &raw);
};

#endif // PROMPTSANITIZER_H
