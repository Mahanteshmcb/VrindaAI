#include "promptsanitizer.h"
#include <QChar>

/* -------------------------
   sanitizePromptForCli: keep printable unicode, preserve newline/tab/CR
   -> do not use regex replacement that triggers invalid-pattern error
   ------------------------- */
QString PromptSanitizer::sanitizePromptForCli(const QString &raw) {
    QString out;
    out.reserve(raw.size());
    for (QChar ch : raw) {
        if (ch == QChar('\n') || ch == QChar('\r') || ch == QChar('\t')) {
            out.append(ch);
            continue;
        }
        // Accept printable chars (unicode safe)
        if (ch.isPrint()) {
            out.append(ch);
        } else {
            out.append(' ');
        }
    }
    // Trim at ends but do not collapse internal newlines
    return out.trimmed();
}

