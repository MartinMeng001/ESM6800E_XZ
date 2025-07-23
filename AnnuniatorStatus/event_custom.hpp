#ifndef EVENT_CUSTOM_HPP
#define EVENT_CUSTOM_HPP
/*关于event的处理
event的处理至少要做到以下几件事情：
1、event触发时能够有效检测
2、event触发后，能够表达已处理状态
3、同一event不会重复触发
*/
#define TRIGGERSTATUS_CHANGE2ON		1
#define TRIGGERSTATUS_CHANGE2OFF	2
#define TRIGGERSTATUS_NOCHANGE		0

#define TRIGGERCOUNT    100
class Event{
public:
    Event(){
        currentStatus=0;	// 存储当前状态，用于判断是否存在状态变化事件
        triggerStatus=TRIGGERSTATUS_NOCHANGE;	// 存储状态改变事件，存在3个值，初始状态，上一事件为变高，上一事件为变低
        beProcessed=true;	// 存储当前事件是否被处理
        counts4EventON = TRIGGERCOUNT;
    }
    // 接口：设置当前状态
    int setCurrentStatus(int status){
        return eventTriggerLogic(status);
    }
    int setInitStatus(int status){
        currentStatus=status;	// 存储当前状态，用于判断是否存在状态变化事件
        triggerStatus=TRIGGERSTATUS_NOCHANGE;	// 存储状态改变事件，存在3个值，初始状态，上一事件为变高，上一事件为变低
        beProcessed=true;	// 存储当前事件是否被处理
        return 0;
    }
    // 接口：获取未被处理的事件
    int getEvent(){
        if(beProcessed==false){
            setEventProcessed();
            return triggerStatus;
        }
        return TRIGGERSTATUS_NOCHANGE;
    }
    // 防止误触发逻辑：
    // 断电后，要连续检测到1000（阈值）断电，才触发断电事件
    // 上电事件不受此影响
    bool eventOnLogic(int status){
        bool ret = false;
        if(status==1){
            if(counts4EventON<=0) ret=true;
            else{
                 counts4EventON--;
            }
        }else{
            counts4EventON = TRIGGERCOUNT;
        }
        return ret;
    }
protected:
    // 基本事件判断逻辑方法
    int eventTriggerLogic(int status){
        int trigger = checkCurrentStatus(status);
        if(trigger==TRIGGERSTATUS_NOCHANGE) return 0;
        int event = checkTriggerStatus(trigger);
        if(event==0) return 0;
        setWaitProcessing();
        return 1;
    }
    // 判断状态是否改变
    int checkCurrentStatus(int status){
        bool allowTrigger = eventOnLogic(status);
        if(status!=currentStatus) {
            if(status==1 && allowTrigger==false)return TRIGGERSTATUS_NOCHANGE;//不允许触发断电事件，则不触发
            currentStatus = status;
            if(currentStatus==1) return TRIGGERSTATUS_CHANGE2ON;
            else return TRIGGERSTATUS_CHANGE2OFF;
        }
        return TRIGGERSTATUS_NOCHANGE;
    }
    // 判断是否触发事件
    int checkTriggerStatus(int statusChange){
        if(statusChange == TRIGGERSTATUS_NOCHANGE) return 0;
        if(triggerStatus!=statusChange){
            triggerStatus=statusChange;
            return 1;
        }
        return 0;
    }

    // 设置存在未处理事件
    void setWaitProcessing(){beProcessed=false;}
    // 设置事件已处理
    void setEventProcessed(){beProcessed=true;}
private:
    int currentStatus;	// 存储当前状态，用于判断是否存在状态变化事件
    int triggerStatus;	// 存储状态改变事件，存在3个值，初始状态，上一事件为变高，上一事件为变低
    bool beProcessed;	// 存储当前事件是否被处理
    int counts4EventON;
};
#endif // EVENT_CUSTOM_HPP
