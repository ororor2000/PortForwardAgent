#include "mongoose.h"

#define SIZE 100

static const char *s_url = "ws://localhost:8080";

// Print websocket response and signal that we're done
static void fn(struct mg_connection *c, int ev, void *ev_data, void *fn_data) 
{
  bool* open = (bool*)(fn_data);
  bool* wait_for_reply = (bool*)(fn_data + 1);

  if (ev == MG_EV_ERROR) 
  {
    MG_ERROR(("%p %s", c->fd, (char *) ev_data));
  }
  else if (ev == MG_EV_WS_OPEN) 
  {
    *open = true;
    mg_ws_send(c, "hello", 5, WEBSOCKET_OP_TEXT);
  }
  else if (ev == MG_EV_WS_MSG) 
  {
    // When we get echo response, print it
    struct mg_ws_message *wm = (struct mg_ws_message *) ev_data;
    printf("REPLY: %s\n", wm->data.ptr);

    *wait_for_reply = false;
  }


  if (ev == MG_EV_ERROR || ev == MG_EV_CLOSE) {
    *open = false;  // Signal that we're done
  }
}

int main(void) {
  struct mg_mgr mgr;
  struct mg_connection *c;

  bool done = false;
  bool flags[2] = {0};
  char msg[SIZE];

  mg_mgr_init(&mgr);
  //mg_log_set(MG_LL_DEBUG);
  c = mg_ws_connect(&mgr, s_url, fn, flags, NULL);

  while (c && done == false) 
  {
    mg_mgr_poll(&mgr, 1000);

    if (!flags[1])
    { 
      bzero(msg, SIZE);
      printf("enter mgs: ");
      fgets(msg, SIZE, stdin);

      mg_ws_send(c, msg, SIZE, WEBSOCKET_OP_TEXT);
      *(flags + 1) = true;
    }
  }

  mg_mgr_free(&mgr);
  return 0;
}