# Simple Character Driver

## Initializing module

```
make
sudo insmod chr_driver.ko

cd /dev
sudo mknod m_device c 245 0
```

## Removing module

```
sudo rmmod chr_driver.ko
sudo rm -rf /dev/m_device
```

## Study references

[Linux Kernel Labs](https://linux-kernel-labs.github.io/refs/heads/master/)
[The Linux Kernel Module Programming Guide](https://tldp.org/LDP/lkmpg/2.6/lkmpg.pdf)
