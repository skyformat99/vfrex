#ifndef WINDOWS_H
#define WINDOWS_H

#include <QtGui>

class Windows : public QWidget
{
    Q_OBJECT
public:
    explicit Windows(QWidget *parent = 0);
    void setSourceModel(QAbstractItemModel *model);

private:
    QGroupBox *sourceGroupBox;
    QTreeView *sourceView;
    QCheckBox *caseSensitivityCheckBox;
    QLabel *rootDirLabel;
    QLineEdit *rootDirLineEdit;
    QLabel *filterPatternLabel;
    QLineEdit *filterPatternLineEdit;
    QLabel *fileFilterLabel;
    QLineEdit *fileFilterLineEdit;
    QCheckBox *caseCheckBox;

    QStandardItemModel *model;

    int ignore_case;

    bool valid_file(const QString &);
    void findAllFile(const QString &);
    void grep(const QString &file_name);

private slots:
    void filterRegExpChanged();
    void caseChanged();

};

#endif // WINDOWS_H
