#include "dialogplanedit.h"
#include "ui_dialogplanedit.h"
#include <QTextStream>
#include <QFileDialog>
#include <iostream>
#include <opencv2/opencv.hpp>

DialogPlanEdit::DialogPlanEdit(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogPlanEdit)
{
    ui->setupUi(this);

    connect(ui->mat_Display, SIGNAL(sendMousePosition(QPoint&)), this, SLOT(showMousePosition(QPoint&)));
}

DialogPlanEdit::~DialogPlanEdit()
{
    delete ui;
}

void DialogPlanEdit::showMousePosition(QPoint &pos)
{
    //shows mouse position in real world coordinates with respect to camera position
    int h_max = ui->mat_Display->size().height();
    int w_max = ui->mat_Display->size().width();


    float y_new = (float)(h_max - pos.y()) / h_max * ui->mat_Display->plan_height - (float)(h_max - ui->mat_Display->camera_pos.y()) / h_max * ui->mat_Display->plan_height;

    float x_new = (float)pos.x() / w_max * ui->mat_Display->plan_width - (float)ui->mat_Display->camera_pos.x() / w_max * ui->mat_Display->plan_width;

    ui->mouse_position_label->setText("x: "+ QString::number(x_new) + " ,y: " + QString::number(y_new));

}


void DialogPlanEdit::on_pushButton_clicked()
{
    QString file_name = QFileDialog::getOpenFileName(this, "Choose plan", "/home");
    QTextStream out(stdout);
    out << file_name << Qt::endl;
    ui->mat_Display->setFixedHeight(490);
    ui->mat_Display->setFixedWidth(790);
    ui->mat_Display->image_current = new QImage;
    ui->mat_Display->image_current->load(file_name);
    ui->mat_Display->floorPlanDirectory = file_name;
    ui->mat_Display->update();
}

void DialogPlanEdit::on_pushButton_2_clicked()
{
    ui->mat_Display->clear_selections();
    ui->mat_Display->update();
}

void DialogPlanEdit::on_pushButton_3_clicked()
{
    ui->mat_Display->activate_place_camera();

}

void DialogPlanEdit::on_height_edit_returnPressed()
{
    QTextStream out(stdout);
    float height = ui->height_edit->text().toFloat();
    out <<  height << Qt::endl;
    ui->mat_Display->plan_height = height;

}

void DialogPlanEdit::on_width_edit_returnPressed()
{
    QTextStream out(stdout);
    float width = ui->width_edit->text().toFloat();
    out <<  width << Qt::endl;
    ui->mat_Display->plan_width = width;
}

// save on file
void DialogPlanEdit::writeVectorOfVector(cv::FileStorage &fs, std::string name, std::vector<std::vector<int>> &data)
{
    int h_max = ui->mat_Display->size().height();
    int w_max = ui->mat_Display->size().width();

    fs << name;
    fs << "{";
    for (int i = 0; i < data.size(); i++)
    {
        fs << name + "_" + std::to_string(i);
        std::vector<float> tmp;
        tmp.push_back((float)data[i][0] / w_max * ui->mat_Display->plan_width - (float)ui->mat_Display->camera_pos.x() / w_max * ui->mat_Display->plan_width);
        tmp.push_back((float)(h_max - data[i][1]) / h_max * ui->mat_Display->plan_height - (float)(h_max - ui->mat_Display->camera_pos.y()) / h_max * ui->mat_Display->plan_height);
        tmp.push_back(((float)(data[i][2]+data[i][0]) / w_max * ui->mat_Display->plan_width - (float)ui->mat_Display->camera_pos.x() / w_max * ui->mat_Display->plan_width) - tmp[0]);
        tmp.push_back((float)(h_max - (data[i][3] + data[i][1])) / h_max * ui->mat_Display->plan_height - (float)(h_max - ui->mat_Display->camera_pos.y()) / h_max * ui->mat_Display->plan_height - tmp[1]);
        fs << tmp;
    }
    fs << "}";
}

void DialogPlanEdit::on_pushButton_4_clicked()
{
    QTextStream out(stdout);
    out << "Saving" << Qt::endl;
    cv::FileStorage fs("test.yml", cv::FileStorage::WRITE);
    fs << "WidthOfPlan" << ui->mat_Display->plan_width;
    fs << "HeightOfPlan" << ui->mat_Display->plan_height;
    fs << "FloorPlanDirectory" << ui->mat_Display->floorPlanDirectory.toUtf8().constData();;
    writeVectorOfVector(fs, "prohibited_rectangles", ui->mat_Display->rectangles);    
    fs.release();
}
