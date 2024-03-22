#include <stdint.h>
#include "efi.h"
#include "efi_helpers.h"

#define scancode_ESC 0x17


EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *ConOut = NULL;
EFI_SIMPLE_TEXT_INPUT_PROTOCOL *ConIn = NULL;
EFI_HANDLE image = NULL;
EFI_BOOT_SERVICES *BootServices;
EFI_RUNTIME_SERVICES *RuntimeServices;

void init_global_vars(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
  ConOut = SystemTable->ConOut;
  ConIn = SystemTable->ConIn;
  BootServices = SystemTable->BootServices;
  RuntimeServices = SystemTable->RuntimeServices;
  image = ImageHandle;
}

EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{

  init_global_vars(ImageHandle, SystemTable);

  con_reset_output(ConOut);

  con_set_color(ConOut, EFI_WHITE, EFI_LIGHTGRAY);

  // Screen loop
  bool running = true;
  while (running)
  {
    con_clear_screen(ConOut);

    con_output_string(ConOut, u"Hello mom!\r\n");
    // con_output_stringf(ConOut, u"stringf string test...: %s\r\n", u"works");
    // con_output_stringf(ConOut, u"stringf int32 test....: %d\r\n", 123);
    // con_output_stringf(ConOut, u"stringf neg int32 test: %d\r\n", -123);
    // con_output_stringf(ConOut, u"stringf hex test......: %x\r\n", 0x45);
    // con_output_stringf(ConOut, u"stringf hex test2.....: %x\r\n", 0x11223344AABBCCDD);

    con_output_string(ConOut, u"\r\nText Mode Info:\r\n");

    ConDimensions dimensions = con_get_query_dimensions(ConOut);
    SIMPLE_TEXT_OUTPUT_MODE mode_info = con_get_mode_info(ConOut);

    // con_output_stringf(ConOut,
    //                    u"Max Mode: %d\r\n"
    //                    u"Current Mode: %d\r\n"
    //                    u"Attribute: %x\r\n"
    //                    u"CursorColumn: %d\r\n"
    //                    u"CursorRow: %d\r\n"
    //                    u"CursorVisible: %d\r\n"
    //                    u"Columns: %d\r\n"
    //                    u"Rows: %d\r\n\r\n",
    //                    mode_info.MaxMode,
    //                    mode_info.Mode,
    //                    mode_info.Attribute,
    //                    mode_info.CursorColumn,
    //                    mode_info.CursorRow,
    //                    mode_info.CursorVisible,
    //                    dimensions.cols,
    //                    dimensions.rows);

    con_output_string(ConOut, u"Available Text Modes:\r\n");
    for (INT32 i = 0; i < mode_info.MaxMode; i++)
    {
      con_update_query_dimensions(ConOut, &dimensions.cols, &dimensions.rows);
      con_output_stringf(ConOut, u"Mode #: %d, %dx%d\r\n", i, dimensions.cols, dimensions.rows);
    }

    mode_info = con_get_mode_info(ConOut);
    static UINTN current_mode = 0;
    current_mode = mode_info.Mode;
    con_output_stringf(ConOut, u"\r\nSelect Text Mode # (0-%d): %d", mode_info.MaxMode, current_mode);
    while (1)
    {
      EFI_INPUT_KEY key = con_get_key(ConIn, BootServices);

      // Print key info
      // CHAR16 cbuf[2] = {key.UnicodeChar, u'\0'};
      CHAR16 cbuf[2];
      cbuf[0] = key.UnicodeChar;
      cbuf[1] = u'\0';

      con_output_stringf(ConOut, u"%s", cbuf);

      current_mode = key.UnicodeChar - u'0';
      EFI_STATUS status = ConOut->SetMode(ConOut, current_mode);

      if (EFI_ERROR(status))
      {
        if (status == EFI_DEVICE_ERROR)
        {
          con_output_stringf(ConOut, u"ERROR: %x; DEVICE ERROR\r\n", status);
        }
        else if (status == EFI_UNSUPPORTED)
        {
          con_output_stringf(ConOut, u"ERROR: %x; Mode number was not valid\r\n", status);
        }
      }

      if (key.ScanCode == scancode_ESC)
      {
        con_output_string(ConOut, u"\r\nshutting down\r\n");
        RuntimeServices->ResetSystem(EfiResetShutdown, EFI_SUCCESS, 0, NULL);
        __attribute__((noreturn));
      }
    }

    return EFI_SUCCESS;
  }
}
