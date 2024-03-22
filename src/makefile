gcc:
		x86_64-w64-mingw32-gcc efi.c \
		-std=c17 \
		-Wall \
		-Wextra \
		-Wpedantic \
		-mno-red-zone \
		-MMD \
		-ffreestanding \
		-nostdlib \
		-Wl,--subsystem,10 \
		-e efi_main \
		-o BOOTX64.EFI

# --subsystem,10 <- 10 is the code for EFI_APPLICATION

# clang-example:
# 		clang example.c \
# 		-target x86_64-unknown-linux \
# 		-std=c17 \
# 		-Wall \
# 		-Wextra \
# 		-Wpedantic \
# 		-mno-red-zone \
# 		-ffreestanding \
# 		-nostdlib \
# 		-fuse-ld=lld-link \
# 		-Wl,-subsystem:efi_application \
# 		-Wl,-entry:efi_main \
# 		-o BOOTX64.EFI

clean:
	 rm *.EFI *.INF *.hdd *.d
