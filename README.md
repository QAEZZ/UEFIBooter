Making a simple UEFI bootloader/manager following Queso Fuego's UEFI dev series. \
I do plan on making a bootloader/manager truly my own without following a series (to an extent, there is a UEFI standard to follow).

**Dev tools needed:**
- qemu x86_64
- edk2-ovmf
- make
- x86_64-w64-mingw32-gcc
- `queso-fuego/UEFI-GPT-image-creator`

### Progress

**Day 2** 
<details>
    <summary>
    Simple block IO disk and partition viewer.
    </summary>
    <img src="./media/blockio-day2.png" alt="day 2 - block io disk and partition info"/>
</details>
<br/>
<details>
    <summary>
    Started work on the boot selector.
    </summary>
    <img src="./media/start-of-boot-selector-day2.png" alt="day 2 - start of boot sel" />
</details>

**Day 1** \
![Day 1 GIF](./media/day1.gif)