# wafel_sysprod_copy
Stroopwafel plugin to copy `sys_prod.xml` from SD root to `slc:/sys/config`

### How to use
- Copy `sys_prod.xml` to the SD Root.
- Download the plugin from the releases
- Copy it to `sd:/wiiu/ios_plugins`.
- Run `Patch (sd) and boot (slc) ios`.
- The sys_prod should have been copied and the plugin deleting itself.

### How to build
```
# define where stroopwafel is
export STROOPWAFEL_ROOT=/path/to/stroopwafel

make
```
