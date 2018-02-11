#include "ttype.h"
#include "ffileio.h"
#include "ccontainer.h"

/*
check available controllers by index. if they are not in the controller list, send event to add to list.
if disconnected, send event
*/

struct Controller{
  FileHandle fd;
  u32 id;//corresponds to js($n)
};

typedef u32 ControllerEvent;

_declare_list(ControllerList,Controller);

void CRefreshControllers(ControllerList controller_list);

ControllerEvent CWaitForControllerEvent(Controller controller);
