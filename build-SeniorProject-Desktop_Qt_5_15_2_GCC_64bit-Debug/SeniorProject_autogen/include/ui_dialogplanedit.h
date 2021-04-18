/********************************************************************************
** Form generated from reading UI file 'dialogplanedit.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DIALOGPLANEDIT_H
#define UI_DIALOGPLANEDIT_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <matdisplay.h>

QT_BEGIN_NAMESPACE

class Ui_DialogPlanEdit
{
public:
    matDisplay *mat_Display;
    QPushButton *pushButton_4;
    QLabel *label_2;
    QLabel *label;
    QPushButton *pushButton;
    QLabel *mouse_position_label;
    QPushButton *pushButton_3;
    QPushButton *pushButton_2;
    QLineEdit *height_edit;
    QLineEdit *width_edit;

    void setupUi(QDialog *DialogPlanEdit)
    {
        if (DialogPlanEdit->objectName().isEmpty())
            DialogPlanEdit->setObjectName(QString::fromUtf8("DialogPlanEdit"));
        DialogPlanEdit->resize(993, 837);
        mat_Display = new matDisplay(DialogPlanEdit);
        mat_Display->setObjectName(QString::fromUtf8("mat_Display"));
        mat_Display->setGeometry(QRect(50, 50, 921, 601));
        pushButton_4 = new QPushButton(DialogPlanEdit);
        pushButton_4->setObjectName(QString::fromUtf8("pushButton_4"));
        pushButton_4->setGeometry(QRect(830, 690, 81, 41));
        pushButton_4->setFocusPolicy(Qt::NoFocus);
        pushButton_4->setAutoDefault(true);
        label_2 = new QLabel(DialogPlanEdit);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(180, 730, 101, 17));
        label = new QLabel(DialogPlanEdit);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(180, 690, 101, 17));
        pushButton = new QPushButton(DialogPlanEdit);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));
        pushButton->setGeometry(QRect(580, 690, 101, 41));
        pushButton->setFocusPolicy(Qt::NoFocus);
        mouse_position_label = new QLabel(DialogPlanEdit);
        mouse_position_label->setObjectName(QString::fromUtf8("mouse_position_label"));
        mouse_position_label->setGeometry(QRect(400, 690, 171, 41));
        mouse_position_label->setStyleSheet(QString::fromUtf8("background-color: rgb(196, 160, 0);"));
        pushButton_3 = new QPushButton(DialogPlanEdit);
        pushButton_3->setObjectName(QString::fromUtf8("pushButton_3"));
        pushButton_3->setGeometry(QRect(60, 690, 111, 41));
        pushButton_3->setFocusPolicy(Qt::NoFocus);
        pushButton_2 = new QPushButton(DialogPlanEdit);
        pushButton_2->setObjectName(QString::fromUtf8("pushButton_2"));
        pushButton_2->setGeometry(QRect(690, 690, 131, 41));
        pushButton_2->setFocusPolicy(Qt::NoFocus);
        height_edit = new QLineEdit(DialogPlanEdit);
        height_edit->setObjectName(QString::fromUtf8("height_edit"));
        height_edit->setGeometry(QRect(270, 690, 113, 25));
        height_edit->setFocusPolicy(Qt::ClickFocus);
        width_edit = new QLineEdit(DialogPlanEdit);
        width_edit->setObjectName(QString::fromUtf8("width_edit"));
        width_edit->setGeometry(QRect(270, 730, 113, 25));
        width_edit->setFocusPolicy(Qt::ClickFocus);

        retranslateUi(DialogPlanEdit);

        QMetaObject::connectSlotsByName(DialogPlanEdit);
    } // setupUi

    void retranslateUi(QDialog *DialogPlanEdit)
    {
        DialogPlanEdit->setWindowTitle(QCoreApplication::translate("DialogPlanEdit", "Dialog", nullptr));
        mat_Display->setText(QString());
        pushButton_4->setText(QCoreApplication::translate("DialogPlanEdit", "Save ", nullptr));
        label_2->setText(QCoreApplication::translate("DialogPlanEdit", "Plan Width:", nullptr));
        label->setText(QCoreApplication::translate("DialogPlanEdit", "Plan Height:", nullptr));
        pushButton->setText(QCoreApplication::translate("DialogPlanEdit", "Change Plan", nullptr));
        mouse_position_label->setText(QString());
        pushButton_3->setText(QCoreApplication::translate("DialogPlanEdit", "Place Camera", nullptr));
        pushButton_2->setText(QCoreApplication::translate("DialogPlanEdit", "Clear Selections", nullptr));
    } // retranslateUi

};

namespace Ui {
    class DialogPlanEdit: public Ui_DialogPlanEdit {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DIALOGPLANEDIT_H
