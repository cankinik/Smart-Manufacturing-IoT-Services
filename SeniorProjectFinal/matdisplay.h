#ifndef MATDISPLAY_H
#define MATDISPLAY_H

#include <QLabel>
#include <QObject>
#include <QWidget>
#include <QMouseEvent>
#include <vector>

class matDisplay : public QLabel
{
    Q_OBJECT
public:
    matDisplay(QWidget *parent = 0);
    ~matDisplay();
    QImage *image_current;
    void clear_selections();
    void activate_place_camera();
    float plan_height;
    float plan_width;
    QPoint camera_pos;
    std::vector<std::vector<int>> rectangles;
    QString floorPlanDirectory;
private:
    bool left_click;
    QPoint left_mouse_pos_click;
    QPoint left_remain_clicked;
    bool is_camera_place_mode_on;
    bool is_camera_placed;


protected:
    void mouseMoveEvent(QMouseEvent *mouse_event);
    void mousePressEvent(QMouseEvent *mouse_event);
    void mouseReleaseEvent(QMouseEvent *mouse_event);

    void paintEvent(QPaintEvent *event);
signals:
    void sendMousePosition(QPoint&);
};

#endif // MATDISPLAY_H
