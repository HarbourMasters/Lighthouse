#include "port/Rando/Rando.h"

class ItemQueue {
public:
    static void Process();
    static void Clear();
    static void GiveItem(RandoItemId randoItemId);
    static void SendNotification(RandoItemId randoItemId);
    static void AddCheck(RandoCheckId randoCheckId);
};
