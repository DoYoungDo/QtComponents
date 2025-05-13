#ifndef HIGHLIGHTER_H
#define HIGHLIGHTER_H

#include <QObject>
#include <QSyntaxHighlighter>

class Highlighter : public QSyntaxHighlighter
{
public:
    explicit Highlighter(QObject *parent);
    explicit Highlighter(QTextDocument *parent);

protected:
    bool render(const QString& text, const QString& pattern, const QColor& color);
    bool render(const QString& text, const QString& pattern, QList<QColor> colors);
};


class JsonHighlighter : public Highlighter
{
public:
    explicit JsonHighlighter(QObject *parent);
    explicit JsonHighlighter(QTextDocument *parent);

    // QSyntaxHighlighter interface
protected:
    virtual void highlightBlock(const QString& text) override;
};

class QssHighlighter : public Highlighter
{
public:
    explicit QssHighlighter(QObject *parent);
    explicit QssHighlighter(QTextDocument *parent);

    // QSyntaxHighlighter interface
protected:
    virtual void highlightBlock(const QString& text) override;
};

class CppHighlighter : public Highlighter
{
public:
    explicit CppHighlighter(QObject *parent);
    explicit CppHighlighter(QTextDocument *parent);

    // QSyntaxHighlighter interface
protected:
    virtual void highlightBlock(const QString& text) override;

private:
    bool startAnnotation = false;
};

#endif // HIGHLIGHTER_H
