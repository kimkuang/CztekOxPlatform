#ifndef MISCUIUTILS_H
#define MISCUIUTILS_H
#include "library_global.h"
#include <QtWidgets>

class LIBRARY_API MiscUiUtils
{
public:
    static void ListItemSwap(QListWidget *list, int direction);
    static void SetLabelColorText(QLabel *label, const QString &text, const QColor &color, bool immediatly = false);
    static void LabelSetText(QLabel *label, const QString &text, const QString &styleSheet, bool immediatly = false);
    static void SetBackground(QWidget *widget, const QColor &color);
    static void SetWidgetColor(QWidget *widget, QPalette::ColorRole role, const QColor &color);
    static void Color2String(QString &str, const QColor &color);
    static bool String2Color(QColor &color, const QString &str);
    static bool CheckLineEditTextIsFloat(QLineEdit *lineEdit, float &val);
    static void InitTableView(QTableView *tabView, QAbstractTableModel *tabModel);
};

#endif // MISCUIUTILS_H
