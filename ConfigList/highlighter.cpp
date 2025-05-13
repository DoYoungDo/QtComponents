#include "highlighter.h"

#include <QRegularExpression>

Highlighter::Highlighter(QObject *parent)
    : QSyntaxHighlighter{parent}
{}

Highlighter::Highlighter(QTextDocument* parent)
    :QSyntaxHighlighter(parent)
{

}


bool Highlighter::render(const QString& text, const QString& pattern, const QColor& color)
{
    return render(text, pattern, QList<QColor>() << color);
}

bool Highlighter::render(const QString& text, const QString& pattern, QList<QColor> colors)
{
    bool has = false;
    QRegularExpression expression(pattern);
    QRegularExpressionMatchIterator i = expression.globalMatch(text);
    while (i.hasNext()) {
        has = true;
        QRegularExpressionMatch match = i.next();
        if(colors.size() == 1)
        {
            setFormat(match.capturedStart(), match.capturedLength(), colors.at(0));
        }
        else if(colors.size() > 1)
        {
            for(int i = 0;i < colors.size();++i)
            {
                QColor color = colors[i];
                setFormat(match.capturedStart(i + 1), match.capturedLength(i + 1), color);
            }
        }
    }
    return has;
}

JsonHighlighter::JsonHighlighter(QObject* parent)
    :Highlighter(parent)
{

}

JsonHighlighter::JsonHighlighter(QTextDocument* parent)
    :Highlighter(parent)
{

}

void JsonHighlighter::highlightBlock(const QString& text)
{
    render(text, "[\\{\\}]", QColor("#b05ebb"));
    render(text, "[\\[\\]]", QColor("#f2d62c"));
    render(text, "\\d", QList<QColor>() << QColor("#b0c59f"));
    render(text, "(true)|(false)", QList<QColor>() << QColor("#517eb1"));
    render(text, "(\".+\")", QList<QColor>() << QColor("#be896f"));
    render(text, "(\".+\")\\s?:", QList<QColor>() << QColor("#98ccf1") << QColor(Qt::white));

}

QssHighlighter::QssHighlighter(QObject* parent)
    :Highlighter(parent)
{

}

QssHighlighter::QssHighlighter(QTextDocument* parent)
    :Highlighter(parent)
{

}

void QssHighlighter::highlightBlock(const QString& text)
{
    render(text, "([\\{\\}])|([\\[\\]])", QColor("#b05ebb"));
    render(text, "[\\(\\)]", QColor("#4b8af8"));
    render(text, "[-:]", QColor("#a4d3fc"));
    render(text, "\\#\\w+\\b", QColor("#cab576"));
    render(text, "\\b\\d+px\\b", QColor("#ba8a6f"));
    render(text, "(background)|(color)|(border)|(height)|(width)|(padding)|(margin)|(left)|(top)|(right)|(bottom)|(image)|(url)", QColor("#a4d3fc"));
    render(text, "(\\b\\w+\\b)(=)('\\w+')", QList<QColor>() << QColor("#a4d3fc") << QColor(Qt::lightGray) << QColor("#ba8a6f"));
    render(text, "/\\*.*\\*/", QList<QColor>() << QColor("#5e9747"));
}

CppHighlighter::CppHighlighter(QObject* parent)
    :Highlighter(parent)
{

}

CppHighlighter::CppHighlighter(QTextDocument* parent)
    :Highlighter(parent)
{

}

void CppHighlighter::highlightBlock(const QString& text)
{
    if(render(text, "\\/\\*.*", QColor("#5e9747")))
    {
        if(!render(text, ".*\\*\\/", QColor("#5e9747")))
        {
            startAnnotation = true;
        }
        return;
    }
    if(render(text, ".*\\*\\/", QColor("#5e9747")))
    {
        startAnnotation = false;
        return;
    }
    if(startAnnotation)
    {
        setFormat(0, text.length(), QColor("#5e9747"));
        return;
    }

    render(text, "[\\{\\}\\[\\]\\<\\>\\(\\):=\\*\\+&\\|]", QColor("#b05ebb"));
    render(text, "\\b(class|struct|enum|typedef|static|private|public|protected|explicit|virtual|void|bool|true|false|return|emit|auto)\\b", QColor("#4b8af8"));
    render(text, "#ifndef|#define|#endif", QColor("#b05ebb"));
    render(text, "(\".+\")", QList<QColor>() << QColor("#be896f"));
    render(text, "(#include) ([\\<\"].+[\\>\"])", QList<QColor>() << QColor("#b05ebb") << QColor("#d69545"));
    render(text, "(\\b\\w+\\b)(\\.|\\-\\>)(\\b\\w+\\b)", QList<QColor>() << QColor("#d6bb9a") << QColor("#d6bb9a") << QColor("#d6cf9a"));
    render(text, "\\b(\\w+)\\b::\\b(\\w+)\\b", QList<QColor>() << QColor("#ff8080") << QColor("#d6cf9a"));
    render(text, "\\/\\/.*", QList<QColor>() << QColor("#5e9747"));
}
