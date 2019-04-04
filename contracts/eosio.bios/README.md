# gxc.bios

system contract that performs a special action.

## Actions

### init

```c++
void init();
```
action to create the necessary users.

**Available Accounts**

|Name|Default|
|----|-------|
|gxc.game||
|gxc.user||
|gxc.reserve||
|gxc.token|privileged: ture|

### setprivs

```c++
void setprivs();
```

Change the account priviliged option.
