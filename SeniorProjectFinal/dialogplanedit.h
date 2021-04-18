#ifndef DIALOGPLANEDIT_H
#define DIALOGPLANEDIT_H

#include <QDialog>
#include <iostream>
#include <opencv2/opencv.hpp>

namespace Ui {
class DialogPlanEdit;
}

class DialogPlanEdit : public QDialog
{
    Q_OBJECT

public:
    explicit DialogPlanEdit(QWidget *parent = nullptr);
    ~DialogPlanEdit();
    void writeVectorOfVector(cv::FileStorage &fs, std::string name, std::vector<std::vector<int> > &data);
private:
    Ui::DialogPlanEdit *ui;
public slots:
    void showMousePosition(QPoint& pos);
private slots:
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
    void on_pushButton_3_clicked();
    void on_height_edit_returnPressed();
    void on_width_edit_returnPressed();
    void on_pushButton_4_clicked();
};

#endif // DIALOGPLANEDIT_H
