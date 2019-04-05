# gxc.bios

system contract that performs a special action.

## Actions

### init

```c++
void init();
```

action to create the necessary users

**Available Accounts**

|Name|Defualt|Owner|Active|
|----|-------|-----|------|
|gxc.user||`gxc@active`|`gxc@active`|
|gxc.game||`gxc@active`|`gxc@active`|
|gxc.reserve||`gxc@active`|`gxc@active`|
||||`gxc.reserve@gxc.code`|
|gxc.token| privileged: true|`gxc@active`|`gxc@active`|
||||`gxc.reserve@gxc.code`|
||||`gxc.token@gxc.code`|
