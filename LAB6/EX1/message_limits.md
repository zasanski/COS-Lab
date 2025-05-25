## POSIX Message Queue Default Limits

These values are typically found in the `/proc/sys/fs/mqueue/` directory and can be inspected using the `sysctl` command.

* **Number of existing message queues:**
    * Default value: **256**
    * Corresponds to `fs.mqueue.queues_max`. This is the maximum total number of message queues allowed on the system.

* **Maximum (system-wide) number of messages per queue:**
    * Default value: **10**
    * Corresponds to `fs.mqueue.msg_max`. This is the upper limit for the `mq_maxmsg` attribute of any individual message queue created by a non-privileged process.

* **Maximum (system-wide) message size:**
    * Default value: **8192 bytes** (8 KB)
    * Corresponds to `fs.mqueue.msgsize_max`. This is the upper limit for the `mq_msgsize` attribute of any individual message.

* **Default maximum number of messages in a newly created queue:**
    * Default value: **10**
    * Corresponds to `fs.mqueue.msg_default`. This value is used for `mq_maxmsg` if you create a new message queue without specifying custom attributes.

* **Default maximum message size in a newly created queue:**
    * Default value: **8192 bytes** (8 KB)
    * Corresponds to `fs.mqueue.msgsize_default`. This value is used for `mq_msgsize` if you create a new message queue without specifying custom attributes.


**How to verify:**

On your Linux system by run:

```bash
sysctl fs.mqueue
```

My output:

```bash
fs.mqueue.msg_default = 10
fs.mqueue.msg_max = 10
fs.mqueue.msgsize_default = 8192
fs.mqueue.msgsize_max = 8192
fs.mqueue.queues_max = 256
```
