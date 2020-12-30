#ifndef HEADSTRUCT_H
#define HEADSTRUCT_H

/*Description
 *command receive from TCP
 *codeId = priority
 *left_speed = left wheel's speed(cm/s)
 *right_speed = right wheel's speed
 *time_ms = command last time(ms)
*/
typedef struct _M_CMD{
    int codeId;
    double left_speed;
    double right_speed;
    int time_ms;
}CMD;

//test
//typedef struct _命令{
//    int 速度;
//    int 时间;
//    int 序号;
//    int 优先级;
//}指令 ;

#endif // HEADSTRUCT_H
