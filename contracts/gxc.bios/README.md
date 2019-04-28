# gxc.bios

bios contract that performs required actions in boot step.

## Actions

### init

```c++
void init();
```

action to create the necessary accounts.

**Available Accounts**

|Name|Defualt|Owner|Active|
|----|-------|-----|------|
|gxc.user||`gxc@active`|`gxc@active`|
|gxc.game||`gxc@active`|`gxc@active`|
|gxc.reserve||`gxc@active`|`gxc@active`|
||||`gxc.reserve@gxc.code`|
|gxc.token|privileged: true|`gxc@active`|`gxc@active`|
||||`gxc.reserve@gxc.code`|
||||`gxc.token@gxc.code`|
