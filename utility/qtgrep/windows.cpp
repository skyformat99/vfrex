#include <QtGui>
#include "windows.h"
#include <windows.h>
#include "vfrex.h"

bool is_binary(const QString &file)
{
    static char buffer[512];
    FILE *fin = fopen(file.toLocal8Bit().data(), "rb");
    if (!fin)
        return true;
    size_t size = fread(buffer, 1, 512, fin);
    for (size_t i = 0; i < size; ++i)
        if (buffer[i] == 0) {
            fclose(fin);
            return true;
        }
    fclose(fin);
    return false;
}


void addEntry(QAbstractItemModel *model, const QString &path, int line, const QString &data)
{
    model->insertRow(0);
    model->setData(model->index(0, 0), path);
    model->setData(model->index(0, 1), line);
    model->setData(model->index(0, 2), data);
}

void Windows::grep(const QString &file_name)
{
    qDebug() << file_name << endl;
    static char buffer[512];
    int line = 0;
    FILE *fin = fopen(file_name.toLocal8Bit().data(), "r");
    for (;;) {
        ++line;
        fgets(buffer, 512, fin);
        if (feof(fin))
            break;
        vfrex_option_t option = {
            REGEX_STYLE_POSIX,
            REGEX_MATCH_PARTIAL_BOUNDARY,
            ignore_case
        };

        QByteArray ba = filterPatternLineEdit->text().toLocal8Bit();
        vfrex_t result = vfrex_match(buffer, ba.data(), option);
        if (result) {
            fprintf(stderr, "Find %s at %d\n", file_name.toLocal8Bit().data(), line);
            addEntry(model, file_name, line, QString(buffer));
            vfrex_free(&result);
        }
    }
    fclose(fin);
}

bool Windows::valid_file(const QString &file_name)
{
    QString filters = fileFilterLineEdit->text();

    foreach(const QString &filter, filters.split(','))
        if (file_name.contains(filter))
            return true;
    return false;
}

void Windows::findAllFile(const QString &path) {
    if (path == "")
        return;
    QDir dir(path);
    qDebug() << path << endl;
    if (!dir.exists()) {
        fprintf(stderr, "failed to find %s\n", path.toLocal8Bit().data());
        return;
    }

    dir.setFilter(QDir::Dirs|QDir::Files);
    QFileInfoList list = dir.entryInfoList();
    for (int i = 0; i < list.size(); ++i) {
        QFileInfo fileInfo = list[i];
        if (fileInfo.fileName() == "." ||
            fileInfo.fileName() == "..") {
            continue;
        }
        if (fileInfo.isDir())
            findAllFile(fileInfo.filePath());
        else {
            if (valid_file(fileInfo.filePath().toLocal8Bit().data())) {
                if (is_binary(fileInfo.filePath()))
                    continue;
                grep(fileInfo.filePath());
            }
        }
    }
}

Windows::Windows(QWidget *parent): QWidget(parent)
{
    sourceView = new QTreeView;
    sourceView->setUniformRowHeights(true);
    sourceView->setRootIsDecorated(false);
    sourceView->setAlternatingRowColors(true);
    sourceView->setSortingEnabled(true);

    rootDirLineEdit = new QLineEdit;
    rootDirLabel = new QLabel(tr("Root Direction:"));
    rootDirLabel->setBuddy(rootDirLabel);

    fileFilterLineEdit = new QLineEdit;
    fileFilterLabel = new QLabel(tr("Filename Filter:"));
    fileFilterLabel->setBuddy(fileFilterLabel);

    filterPatternLineEdit = new QLineEdit;
    filterPatternLabel = new QLabel(tr("Regular Expression:"));
    filterPatternLabel->setBuddy(filterPatternLabel);

    caseCheckBox = new QCheckBox("Ignore case");

    sourceGroupBox = new QGroupBox(tr("Online Regex Finder"));


    QGridLayout *sourceLayout = new QGridLayout;
    sourceLayout->addWidget(rootDirLabel, 0, 0);
    sourceLayout->addWidget(rootDirLineEdit, 0, 1, 1, 5);
    sourceLayout->addWidget(fileFilterLabel, 1, 0);
    sourceLayout->addWidget(fileFilterLineEdit, 1, 1, 1, 4);
    sourceLayout->addWidget(caseCheckBox, 1, 5, 1, 1);
    sourceLayout->addWidget(filterPatternLabel, 2, 0);
    sourceLayout->addWidget(filterPatternLineEdit, 2, 1, 1, 5);
    sourceLayout->addWidget(sourceView, 3, 0, 1, 6);
    sourceGroupBox->setLayout(sourceLayout);

    QVBoxLayout *mainLayout = new QVBoxLayout;

    mainLayout->addWidget(sourceGroupBox);

    setLayout(mainLayout);

    setWindowTitle(tr("Demo For Regular Expression"));
    resize(800, 600);

    sourceView->sortByColumn(1, Qt::AscendingOrder);
    sourceView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    rootDirLineEdit->setText("D:\\QtSDK\\Examples\\4.7\\itemviews\\basicsortfiltermodel");
    filterPatternLineEdit->setText("Andy|Grace");

    connect(filterPatternLineEdit, SIGNAL(textChanged(QString)),
            this, SLOT(filterRegExpChanged()));
    connect(rootDirLineEdit, SIGNAL(textChanged(QString)),
            this, SLOT(filterRegExpChanged()));
    connect(fileFilterLineEdit, SIGNAL(textChanged(QString)),
            this, SLOT(filterRegExpChanged()));
    connect(caseCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(caseChanged()));

    model = new QStandardItemModel(0, 3);
    model->setHeaderData(0, Qt::Horizontal, QObject::tr("Path"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Line"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Content"));

    setSourceModel(model);
    filterRegExpChanged();
    ignore_case = 0;
}


void Windows::setSourceModel(QAbstractItemModel *model)
{
    sourceView->setModel(model);
    sourceView->setColumnWidth(0, 300);
    sourceView->setColumnWidth(1, 50);
}

void Windows::filterRegExpChanged()
{
    model->removeRows(0, model->rowCount());
    findAllFile(rootDirLineEdit->text());
    setSourceModel(model);
}

void Windows::caseChanged()
{
    ignore_case = caseCheckBox->isChecked();
    filterRegExpChanged();
}
