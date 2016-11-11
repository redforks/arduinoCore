#include "core.h"
#include <stdarg.h>

namespace core {
struct Node {
  callback f;
  Node *next;

  unsigned long mills, interval;  // only used in clock namespace

  Node(callback fn) : f(fn) {}
  Node(callback fn, unsigned long delay) : f(fn), mills(delay) {}
  Node(callback fn, unsigned long delay, unsigned long anInterval)
      : f(fn), mills(delay), interval(anInterval) {}
};

struct Queue {
  Node *head;

  void add(Node *);
  void clear();
  void invokeAll();
  void appendQueue(Queue);
  void remove(Node *);

 private:
  bool exist(callback f);
};

void Queue::add(Node *node) {
  if (head == NULL) {
    head = node;
    return;
  }

  node->next = head;
  head = node;
}

void Queue::clear() {
  for (Node *p = head; p != NULL;) {
    Node *next = p->next;
    delete (p);
    p = next;
  }
  head = NULL;
}

void Queue::invokeAll() {
  for (Node *p = head; p != NULL; p = p->next) {
    p->f();
  }
}

// append callbacks, ignored if it exist in Queue.
void Queue::appendQueue(Queue queue) {
  for (Node *p = queue.head; p != NULL; p = p->next) {
    if (!Queue::exist(p->f)) {
      add(new Node(p->f));
    }
  }
}

bool Queue::exist(callback f) {
  for (Node *p = head; p != NULL; p = p->next) {
    if (p->f == f) {
      return true;
    }
  }
  return false;
}

void Queue::remove(Node *node) {
  if (head == node) {
    head = head->next;
    delete (node);
    return;
  }

  for (Node *p = head; p != NULL; p = p->next) {
    if (p->next == node) {
      p->next = node->next;
      delete (node);
      break;
    }
  }
}

namespace store {
// Current value of digitals.
byte digitals[DIGITAL_VALUES];

// Current value of the analogs.
word analogs[ANALOG_VALUES];
Queue digitalMonitors[DIGITAL_VALUES];
Queue analogMonitors[ANALOG_VALUES];

idType digitalIdSeed = 0;
idType analogIdSeed = 0;

idType defineDigital() { return digitalIdSeed++; }

idType defineAnalog() { return analogIdSeed++; }

void monitorDigitals(callback f, byte nIds, ...) {
  va_list ap;
  va_start(ap, nIds);
  for (int i = 0; i < nIds; i++) {
    idType id = va_arg(ap, int);
    Node *node = new Node(f);
    store::digitalMonitors[id].add(node);
  }
}

void monitorAnalogs(callback f, byte nIds, ...) {
  va_list ap;
  va_start(ap, nIds);
  for (int i = 0; i < nIds; i++) {
    idType id = va_arg(ap, int);
    Node *node = new Node(f);
    store::analogMonitors[id].add(node);
  }
}

byte inBatch = 0;
Queue dirtyCallbacks;  // callbacks need trigger when batch end.
void beginBatchUpdate() { inBatch++; }

void endBatchUpdate() {
  inBatch--;
  if (inBatch == 0) {
    dirtyCallbacks.invokeAll();
    dirtyCallbacks.clear();
  }
}

void setDigital(idType id, byte val) {
  if (digitals[id] == val) {
    return;
  }

  digitals[id] = val;

  if (inBatch == 0) {
    digitalMonitors[id].invokeAll();
    return;
  }

  dirtyCallbacks.appendQueue(digitalMonitors[id]);
}

void setAnalog(idType id, word val) {
  if (analogs[id] == val) {
    return;
  }

  analogs[id] = val;

  if (inBatch == 0) {
    analogMonitors[id].invokeAll();
    return;
  }

  dirtyCallbacks.appendQueue(analogMonitors[id]);
}
}

namespace clock {

Queue intervals, delays;

void *interval(unsigned long mills, callback f) {
  Node *node = new Node(f, mills + millis(), mills);
  intervals.add(node);
  return node;
}

void *delay(unsigned long mills, callback f) {
  Node *node = new Node(f, mills + millis());
  delays.add(node);
  return node;
}

void removeInterval(void *id) { intervals.remove((Node *)id); }

void removeDelay(void *id) { delays.remove((Node *)id); }

void check() {
  unsigned long cur = millis();
  for (Node *p = delays.head; p != NULL; p = p->next) {
    if (cur - p->mills >= 0) {
      delays.remove(p);
      p->f();
    }
  }

  for (Node *p = intervals.head; p != NULL; p = p->next) {
    if (cur - p->mills >= 0) {
      p->mills = cur + p->interval;
      p->f();
    }
  }
}
}
}
