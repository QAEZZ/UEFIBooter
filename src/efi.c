#include <stdint.h>
#include "efi.h"
#include "efi_helpers.h"

EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *ConOut = NULL;
EFI_SIMPLE_TEXT_INPUT_PROTOCOL *ConIn = NULL;
EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *ConErr = NULL;
EFI_HANDLE image = NULL;
EFI_BOOT_SERVICES *BootServices;
EFI_RUNTIME_SERVICES *RuntimeServices;

void init_global_vars(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
  ConOut = SystemTable->ConOut;
  ConIn = SystemTable->ConIn;
  ConErr = SystemTable->StdErr;
  BootServices = SystemTable->BootServices;
  RuntimeServices = SystemTable->RuntimeServices;
  image = ImageHandle;
}

// con_text_info from Queso Feugo's "UEFI Dev (in C)".
EFI_STATUS stats_for_nerds()
{
  con_set_color(ConOut, DEFAULT_FG_COLOR, DEFAULT_BG_COLOR);
  bool running = true;
  while (running)
  {
    con_clear_screen(ConOut);

    con_output_stringf(ConOut, u"stringf string test...: %s\r\n", u"works");
    con_output_stringf(ConOut, u"stringf int32 test....: %d\r\n", 123);
    con_output_stringf(ConOut, u"stringf neg int32 test: %d\r\n", -123);
    con_output_stringf(ConOut, u"stringf hex test......: %x\r\n", 0x45);
    con_output_stringf(ConOut, u"stringf hex test2.....: %x\r\n", 0x11223344AABBCCDD);

    con_output_string(ConOut, u"\r\nText Mode Info:\r\n");

    ConDimensions dimensions = con_get_query_dimensions(ConOut);
    SIMPLE_TEXT_OUTPUT_MODE mode_info = con_get_mode_info(ConOut);

    con_output_stringf(ConOut,
                       u"Max Mode: %d\r\n"
                       u"Current Mode: %d\r\n"
                       u"Attribute: %x\r\n"
                       u"CursorColumn: %d\r\n"
                       u"CursorRow: %d\r\n"
                       u"CursorVisible: %d\r\n"
                       u"Columns: %d\r\n"
                       u"Rows: %d\r\n\r\n",
                       mode_info.MaxMode,
                       mode_info.Mode,
                       mode_info.Attribute,
                       mode_info.CursorColumn,
                       mode_info.CursorRow,
                       mode_info.CursorVisible,
                       dimensions.cols,
                       dimensions.rows);

    con_output_string(ConOut, u"Available Text Modes:\r\n");
    for (INT32 i = 0; i < mode_info.MaxMode; i++)
    {
      con_update_query_dimensions(ConOut, &dimensions.cols, &dimensions.rows);
      con_output_stringf(ConOut, u"Mode #: %d, %dx%d\r\n", i, dimensions.cols, dimensions.rows);
    }

    ConOut->SetCursorPosition(ConOut, 0, dimensions.rows - 1);
    con_output_string(ConOut, u"Esc -> Go back\r");

    ConOut->SetCursorPosition(ConOut, dimensions.cols - sizeof(u"Hello mom!\r"), dimensions.rows - 1);
    con_output_string(ConOut, u"Hello mom!\r");
    while (true)
    {
      EFI_INPUT_KEY key = con_get_key(ConIn, BootServices);

      CHAR16 cbuf[2];
      cbuf[0] = key.UnicodeChar;
      cbuf[1] = u'\0';

      if (key.ScanCode == SCANCODE_ESC)
      {
        return EFI_SUCCESS;
      }
    }

    return EFI_SUCCESS;
  }
}

VOID *memset(VOID *dst, UINT8 c, UINTN len)
{
  UINT8 *p = dst;
  for (UINTN i = 0; i < len; i++)
  {
    p[i] = c;
  }
  return dst;
}

VOID *memcpy(VOID *dst, VOID *src, UINT8 c, UINTN len)
{
  UINT8 *p = dst;
  UINT8 *q = src;
  for (UINTN i = 0; i < len; i++)
  {
    p[i] = q[i];
  }
  return dst;
}

INTN *memcmp(VOID *m1, VOID *m2, UINTN len)
{
  UINT8 *p = m1;
  UINT8 *q = m2;
  for (UINTN i = 0; i < len; i++)
  {
    if (p[i] != q[i])
      return (INTN)p[i] - (INTN)q[i];
  }
  return 0;
}

CHAR16 *strcpy_u16(CHAR16 *dst, CHAR16 *src)
{
  if (!dst)
    return NULL;
  if (!src)
    return dst;

  CHAR16 *result = dst;
  while (*src)
    *dst++ = *src++;
  *dst = u'\0';

  return dst;
}

INTN strncmp_u16(CHAR16 *s1, CHAR16 *s2, UINTN len)
{
  if (len == 0)
    return 0;

  while (len > 0 && *s1 && *s2 && *s1 == *s2)
  {
    s1++;
    s2++;
    len--;
  }

  return *s1 - *s2;
}

CHAR16 *strrchr_u16(CHAR16 *str, CHAR16 c)
{
  CHAR16 *result = NULL;

  while (*str)
  {
    if (*str == c)
      result = str;
    str++;
  }

  return result;
}

CHAR16 *strcat_u16(CHAR16 *dst, CHAR16 *src)
{
  CHAR16 *s = dst;

  while (*s)
    s++;

  while (*src)
    *s++ = *src++;

  *s = u'\0';
  return dst;
}

UINTN strlen(CHAR16 *s)
{
  UINTN len = 0;
  while (*s != '\0')
  {
    len++;
    s++;
  }
  return len;
}

void fill_remaining_with_char(UINTN start_col, CHAR16 fill_char, UINTN max_cols)
{
  ConOut->SetCursorPosition(ConOut, start_col, con_get_mode_info(ConOut).CursorRow);

  for (UINTN i = start_col; i < max_cols; i++)
  {
    CHAR16 buffer[2] = {fill_char, u'\0'};
    con_output_string(ConOut, buffer);
  }
}

EFI_STATUS read_esp_files(void)
{
  con_set_color(ConOut, DEFAULT_FG_COLOR, DEFAULT_BG_COLOR);
  con_clear_screen(ConOut);

  EFI_GUID lip_guid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
  EFI_LOADED_IMAGE_PROTOCOL *lip = NULL;
  EFI_STATUS status = EFI_SUCCESS;

  status = BootServices->OpenProtocol(image,
                                      &lip_guid,
                                      (VOID **)&lip,
                                      image,
                                      NULL,
                                      EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);

  if (EFI_ERROR(status))
  {
    con_output_stringf(ConOut,
                       u"ERROR: %x\r\nCould not open Loaded Image Protocol.\r\n",
                       status);
    return status;
  }

  EFI_GUID sfsp_guid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *sfsp = NULL;
  status = BootServices->OpenProtocol(lip->DeviceHandle,
                                      &sfsp_guid,
                                      (VOID **)&sfsp,
                                      image,
                                      NULL,
                                      EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);

  if (EFI_ERROR(status))
  {
    con_output_stringf(ConOut,
                       u"ERROR: %x\r\nCould not open Simple File System Protocol.\r\n");
    return status;
  }

  EFI_FILE_PROTOCOL *dirp = NULL;
  status = sfsp->OpenVolume(sfsp, &dirp);
  if (EFI_ERROR(status))
  {
    con_output_stringf(ConOut,
                       u"ERROR: %x\r\nCould not open Volume for the root dir.\r\n");
    return status;
  }

  CHAR16 current_directory[256];
  strcpy_u16(current_directory, u"/");

  INT32 csr_row = 1;
  while (true)
  {
    con_clear_screen(ConOut);
    con_output_stringf(ConOut, u"Current Dir: %s\r\n", current_directory);

    EFI_FILE_INFO file_info;
    dirp->SetPosition(dirp, 0); // start of dir entries
    UINTN buf_size = sizeof(file_info);
    dirp->Read(dirp, &buf_size, &file_info);
    INT32 num_entries = 0;
    while (buf_size > 0)
    {
      num_entries++;
      if (csr_row == con_get_mode_info(ConOut).CursorRow)
      {
        con_set_color(ConOut, HIGHLIGHT_FG_COLOR, HIGHLIGHT_BG_COLOR);
      }
      con_output_stringf(ConOut, u"%s %s\r\n", (file_info.Attribute & EFI_FILE_DIRECTORY) ? u"[DIR] " : u"[FILE]", file_info.FileName);

      if (csr_row + 1 == con_get_mode_info(ConOut).CursorRow)
      {
        con_set_color(ConOut, DEFAULT_FG_COLOR, DEFAULT_BG_COLOR);
      }

      buf_size = sizeof(file_info);
      dirp->Read(dirp, &buf_size, &file_info);
    }

    con_output_stringf(ConOut, u"Half: %d\r\n", con_get_query_dimensions(ConOut).cols / 2);
    ConOut->SetCursorPosition(ConOut, 0, con_get_query_dimensions(ConOut).rows - 1);
    con_output_string(ConOut, u"Esc -> Go back\r");
    // ConOut->SetCursorPosition(ConOut, con_get_query_dimensions(ConOut).cols - sizeof(), 0);
    ConOut->SetCursorPosition(ConOut, 0, num_entries + 2);

    EFI_INPUT_KEY key = con_get_key(ConIn, BootServices);
    switch (key.ScanCode)
    {
    case SCANCODE_ESC:
      goto cleanup;
      break;
    case SCANCODE_UP_ARROW:
      if (csr_row > 1)
        csr_row--;
      break;
    case SCANCODE_DOWN_ARROW:
      if (csr_row < num_entries)
        csr_row++;
      break;
    default:
      if (key.UnicodeChar == u'\r')
      {
        dirp->SetPosition(dirp, 0);
        INT32 i = 0;
        do
        {
          buf_size = sizeof(file_info);
          dirp->Read(dirp, &buf_size, &file_info);
          i++;
        } while (i < csr_row);

        if (file_info.Attribute & EFI_FILE_DIRECTORY)
        {

          EFI_FILE_PROTOCOL *new_dir;
          status = dirp->Open(dirp,
                              &new_dir,
                              file_info.FileName,
                              EFI_FILE_MODE_READ,
                              0);

          if (EFI_ERROR(status))
          {
            con_output_stringf(ConOut,
                               u"ERROR: %x\r\nCould not open new directory %s\r\n",
                               status,
                               file_info.FileName);
            goto cleanup;
          }

          dirp->Close(dirp);
          dirp = new_dir;
          csr_row = 1;

          if (!strncmp_u16(file_info.FileName, u".", 2))
          {
            // Do nothing
          }
          else if (!strncmp_u16(file_info.FileName, u"..", 3))
          {

            CHAR16 *pos = strrchr_u16(current_directory, u'/');

            if (pos == current_directory)
              pos++;
            // pos++;
            *pos = u'\0';
          }
          else
          {
            if (current_directory[1] != u'\0')
            {
              strcat_u16(current_directory, u"/");
            }
            strcat_u16(current_directory, file_info.FileName);
          }
          continue;
        }

        VOID *file_buffer = NULL;
        buf_size = file_info.FileSize;
        BootServices->AllocatePool(EfiLoaderData, buf_size, &file_buffer);
        status = dirp->Read(dirp, &buf_size, file_buffer);

        if (EFI_ERROR(status))
        {
          con_output_stringf(ConOut,
                             u"ERROR: %x\r\nCould not allocate memory for file %s\r\n",
                             status,
                             file_info.FileName);
          con_output_string(ConOut, u"\r\nPress any key to return...\r\n");
          con_get_key(ConIn, BootServices);
          goto cleanup;
        }

        EFI_FILE_PROTOCOL *file = NULL;
        status = dirp->Open(dirp,
                            &file,
                            file_info.FileName,
                            EFI_FILE_MODE_READ,
                            0);
        if (EFI_ERROR(status))
        {
          con_output_stringf(ConOut,
                             u"ERROR: %x\r\nCould not open file %s\r\n",
                             status,
                             file_info.FileName);
          con_output_string(ConOut, u"\r\nPress any key to return...\r\n");
          con_get_key(ConIn, BootServices);
          goto cleanup;
        }

        buf_size = file_info.FileSize;
        status = dirp->Read(file, &buf_size, file_buffer);
        if (EFI_ERROR(status))
        {
          con_output_stringf(ConOut,
                             u"ERROR: %x\r\nCould not read file %s into buffer\r\n",
                             status,
                             file_info.FileName);
          con_output_string(ConOut, u"\r\nPress any key to return...\r\n");
          con_get_key(ConIn, BootServices);
          goto cleanup;
        }

        if (buf_size != file_info.FileSize)
        {
          con_output_stringf(ConOut,
                             u"ERROR: Could not read all of file %s into buffer\r\n\r\n"
                             u"Bytes read: %d\r\n"
                             u"Expceted..: %d\r\n",
                             file_info.FileName,
                             buf_size,
                             file_info.FileSize);
          con_output_string(ConOut, u"\r\nPress any key to return...\r\n");
          con_get_key(ConIn, BootServices);
          goto cleanup;
        }

        char *pos = (char *)file_buffer;
        // ConOut->SetCursorPosition(ConOut, con_get_query_dimensions(ConOut).cols - sizeof(*pos), 0);

        // status = ConOut->SetCursorPosition(ConOut, 5, 0);
        ConOut->SetCursorPosition(ConOut, con_get_query_dimensions(ConOut).cols / 2, 0);
        con_output_stringf(ConOut, u"Cursor col: %d\r\n", con_get_mode_info(ConOut).CursorColumn);

        ConOut->SetCursorPosition(ConOut, con_get_query_dimensions(ConOut).cols / 2, 1);
        con_output_string(ConOut, u"File Contents:\r\n");

        ConOut->SetCursorPosition(ConOut, con_get_query_dimensions(ConOut).cols / 2, 2);

        for (UINTN character = buf_size; character > 0; character--)
        {
          CHAR16 str[2] = {*pos, u'\0'};
          con_output_string(ConOut, str);
          pos++;
        }

        con_get_key(ConIn, BootServices);

        BootServices->FreePool(file_buffer);
        dirp->Close(file);
      }
      break;
    }
  }

  con_output_string(ConOut, u"\r\nPress any key to return...\r\n");
  con_get_key(ConIn, BootServices);

cleanup:
  dirp->Close(dirp);

  BootServices->CloseProtocol(lip->DeviceHandle,
                              &sfsp_guid,
                              image,
                              NULL);

  BootServices->CloseProtocol(image,
                              &lip_guid,
                              image,
                              NULL);

  return status;
}

EFI_STATUS print_block_io_partitions(void)
{
  EFI_STATUS status = EFI_SUCCESS;
  con_set_color(ConOut, DEFAULT_FG_COLOR, DEFAULT_BG_COLOR);
  con_clear_screen(ConOut);

  EFI_GUID bio_guid = EFI_BLOCK_IO_PROTOCOL_GUID;
  EFI_BLOCK_IO_PROTOCOL *biop;
  UINTN num_handles = 0;
  EFI_HANDLE *handle_buffer = NULL;

  EFI_GUID lip_guid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
  EFI_LOADED_IMAGE_PROTOCOL *lip = NULL;
  status = BootServices->OpenProtocol(image,
                                      &lip_guid,
                                      (VOID **)&lip,
                                      image,
                                      NULL,
                                      EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
  if (EFI_ERROR(status))
  {
    con_output_stringf(ConOut,
                       u"ERROR: %x\r\nCould not open Loaded Image Protocol.\r\n",
                       status);
    con_get_key(ConIn, BootServices);
  }

  status = BootServices->OpenProtocol(lip->DeviceHandle,
                                      &bio_guid,
                                      (VOID **)&biop,
                                      image,
                                      NULL,
                                      EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
  if (EFI_ERROR(status))
  {
    con_output_stringf(ConOut,
                       u"ERROR: %x\r\nCould not open Block IO protocol for this loaded image.\r\n",
                       status);
    con_get_key(ConIn, BootServices);
  }

  UINT32 this_image_media_id = biop->Media->MediaId; // test if can be const

  BootServices->CloseProtocol(lip->DeviceHandle,
                              &bio_guid,
                              image,
                              NULL);
  BootServices->CloseProtocol(image,
                              &lip_guid,
                              image,
                              NULL);

  status = BootServices->LocateHandleBuffer(ByProtocol,
                                            &bio_guid,
                                            NULL,
                                            &num_handles,
                                            &handle_buffer);
  if (EFI_ERROR(status))
  {
    con_output_stringf(ConOut,
                       u"ERROR: %x\r\nCould not locate Block IO Protocols.\r\n",
                       status);
    con_get_key(ConIn, BootServices);
  }

  UINT32 last_media_id = -1;
  for (UINTN i = 0; i < num_handles; i++)
  {
    status = BootServices->OpenProtocol(handle_buffer[i],
                                        &bio_guid,
                                        (VOID **)&biop,
                                        image,
                                        NULL,
                                        EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
    if (EFI_ERROR(status))
    {
      con_output_stringf(ConOut,
                         u"ERROR: %x\r\nCould not open Block IO Protocol on handle %u\r\n",
                         status, i);
      con_get_key(ConIn, BootServices);
    }

    if (last_media_id != biop->Media->MediaId)
    {
      last_media_id = biop->Media->MediaId;
      con_output_string_colored_latter(ConOut,
                                       u"Media ID: ",
                                       (last_media_id == this_image_media_id ? u"(Disk Image)\r\n" : last_media_id + u"\r\n"),
                                       EFI_LIGHTGRAY,
                                       DEFAULT_BG_COLOR);
    }

    // con_output_string_colored_latter(ConOut,
    //                                  u"Removable: ",
    //                                  biop->Media->RemovableMedia ? u"Yes" : u"No",
    //                                  EFI_LIGHTGRAY,
    //                                  DEFAULT_BG_COLOR);
    // con_output_string(ConOut, u" | ");

    // con_output_string_colored_latter(ConOut,
    //                                  u"MediaPresent: ",
    //                                  biop->Media->MediaPresent ? u"Yes" : u"No",
    //                                  EFI_LIGHTGRAY,
    //                                  DEFAULT_BG_COLOR);
    // con_output_string(ConOut, u" | ");

    // con_output_string_colored_latter(ConOut,
    //                                  u"LglPart: ",
    //                                  biop->Media->LogicalPartition ? u"Yes" : u"No",
    //                                  EFI_LIGHTGRAY,
    //                                  DEFAULT_BG_COLOR);
    // con_output_string(ConOut, u" | ");

    // con_output_string_colored_latter(ConOut,
    //                                  u"RdOnly: ",
    //                                  biop->Media->ReadOnly ? u"Yes" : u"No",
    //                                  EFI_LIGHTGRAY,
    //                                  DEFAULT_BG_COLOR);
    // con_output_string(ConOut, u" | ");

    // con_output_string_colored_latter(ConOut,
    //                                  u"WrtCaching: ",
    //                                  biop->Media->WriteCaching ? u"Yes" : u"No",
    //                                  EFI_LIGHTGRAY,
    //                                  DEFAULT_BG_COLOR);
    // con_output_string(ConOut, u" | \r\n");

    // // -- Numerical Data ---------------------------------------------------------

    // con_output_uint32_colored_latter(ConOut,
    //                                  u"BlkSz: ",
    //                                  biop->Media->BlockSize,
    //                                  EFI_LIGHTGRAY,
    //                                  DEFAULT_BG_COLOR);
    // con_output_string(ConOut, u" | ");

    // con_output_uint32_colored_latter(ConOut,
    //                                  u"IoAlign: ",
    //                                  biop->Media->IoAlign,
    //                                  EFI_LIGHTGRAY,
    //                                  DEFAULT_BG_COLOR);
    // con_output_string(ConOut, u" | ");

    // con_output_efi_lba_colored_latter(ConOut,
    //                                  u"LstBlk: ",
    //                                  biop->Media->LastBlock,
    //                                  EFI_LIGHTGRAY,
    //                                  DEFAULT_BG_COLOR);
    // con_output_string(ConOut, u" | ");

    // con_output_efi_lba_colored_latter(ConOut,
    //                                  u"LwLBA: ",
    //                                  biop->Media->LowestAlignedLba,
    //                                  EFI_LIGHTGRAY,
    //                                  DEFAULT_BG_COLOR);
    // con_output_string(ConOut, u" | ");

    // con_output_uint32_colored_latter(ConOut,
    //                                  u"LglBlkPerPhysBlk: ",
    //                                  biop->Media->LogicalBlocksPerPhysicalBlock,
    //                                  EFI_LIGHTGRAY,
    //                                  DEFAULT_BG_COLOR);
    // con_output_string(ConOut, u" | \r\n");

    // con_output_uint32_colored_latter(ConOut,
    //                                  u"OptmlTrnsfrLenGran: ",
    //                                  biop->Media->OptimalTransferLengthGranularity,
    //                                  EFI_LIGHTGRAY,
    //                                  DEFAULT_BG_COLOR);
    // con_output_string(ConOut, u" | \r\n");

    if (biop->Media->LastBlock == 0)
    {
      BootServices->CloseProtocol(handle_buffer[i],
                                  &bio_guid,
                                  image,
                                  NULL);
      continue;
    }

    con_output_stringf(ConOut,
                       u"Removable: %s | MediaPresent: %s | LglPart: %s | RdOnly: %s | WrtCaching: %s\r\n"
                       u"BlkSz: %u | IoAlign: %u | LstBlk: %u | LwLBA: %u | LglBlkPerPhysBlk: %u\r\n"
                       u"OptmlTrnsfrLenGran: %u\r\n",
                       biop->Media->RemovableMedia ? u"Yes" : u"No",
                       biop->Media->MediaPresent ? u"Yes" : u"No",
                       biop->Media->LogicalPartition ? u"Yes" : u"No",
                       biop->Media->ReadOnly ? u"Yes" : u"No",
                       biop->Media->WriteCaching ? u"Yes" : u"No",

                       biop->Media->BlockSize,
                       biop->Media->IoAlign,
                       biop->Media->LastBlock,
                       biop->Media->LowestAlignedLba,
                       biop->Media->LogicalBlocksPerPhysicalBlock,
                       biop->Media->OptimalTransferLengthGranularity);

    if (!biop->Media->LogicalPartition)
    {
      con_output_string(ConOut, u"--<Entire Disk>\r");
      fill_remaining_with_char(strlen(u"--<Entire Disk>"), u'-', con_get_query_dimensions(ConOut).cols);
      con_output_string(ConOut, u"\r\n");
    }
    else
    {
      EFI_GUID pi_guid = EFI_PARTITION_INFO_PROTOCOL_GUID;
      EFI_PARTITION_INFO_PROTOCOL *pip = NULL;
      status = BootServices->OpenProtocol(handle_buffer[i],
                                          &pi_guid,
                                          (VOID **)&pip,
                                          image,
                                          NULL,
                                          EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
      if (EFI_ERROR(status))
      {
        con_output_stringf(ConOut,
                           u"ERROR: %x\r\nCould not open Partition Info Protocol on handle %u\r\n",
                           status, i);
        con_get_key(ConIn, BootServices);
      }
      else
      {
        if (pip->Type == PARTITION_TYPE_MBR)
          con_output_string(ConOut, u"<MBR>\r\n");
        else if (pip->Type == PARTITION_TYPE_OTHER)
          con_output_string(ConOut, u"<OTHER>\r\n");
        else if (pip->Type == PARTITION_TYPE_GPT)
        {
          if (pip->System == 1)
          {
            con_output_string(ConOut, u"--<EFI Sys Partition>\r");
            fill_remaining_with_char(strlen(u"--<EFI Sys Partition>"), u'-', con_get_query_dimensions(ConOut).cols);
            con_output_string(ConOut, u"\r\n");
          }
          else
          {
            EFI_GUID bd_guid = BASIC_DATA_GUID;
            if (!memcmp(&pip->Info.Gpt.PartitionTypeGUID, &bd_guid, sizeof(EFI_GUID)))
            {
              con_output_string(ConOut,
                                u"--<Basic Data>\r");

              fill_remaining_with_char(strlen(u"--<Basic Data>"), u'-', con_get_query_dimensions(ConOut).cols);
              con_output_string(ConOut, u"\r\n");
            }
            else
            {
              con_output_string(ConOut, u"--<Other GPT Type>\r");
              fill_remaining_with_char(strlen(u"--<Other GPT Type>"), u'-', con_get_query_dimensions(ConOut).cols);
              con_output_string(ConOut, u"\r\n");
            }
          }
        }
      }
    }
    con_output_string(ConOut, u"\r\n");
  }

  con_output_string(ConOut, u"Press any key to return...\r\n");
  con_get_key(ConIn, BootServices);
  return status;
}

EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{

  init_global_vars(ImageHandle, SystemTable);

  con_reset_output(ConOut);

  con_set_color(ConOut, DEFAULT_FG_COLOR, DEFAULT_BG_COLOR);

  ConOut->SetMode(ConOut, 2);
  // ConOut->QueryMode(ConOut, 3, )

  const CHAR16 *menu_choices[] = {
      u"Stats For Nerds",
      u"Set Graphics Mode",
      u"Read ESP Files",
      u"Print BLOCK IO Partitions"};

  const EFI_STATUS (*menu_funcs[])(void) = {
      stats_for_nerds,
      system_shutdown,
      read_esp_files,
      print_block_io_partitions};

  // Screen loop
  bool running = true;
  while (running)
  {
    con_clear_screen(ConOut);

    ConDimensions dimensions = con_get_query_dimensions_with_mode(ConOut, ConOut->Mode->Mode);
    ConOut->SetCursorPosition(ConOut, 0, dimensions.rows - 3);
    con_output_string(ConOut, u"Up/Down Arrow -> Move\r\n"
                              u"Enter ---------> Select\r\n"
                              u"Esc -----------> Shutdown");

    ConOut->SetCursorPosition(ConOut, 0, 0);
    con_set_color(ConOut, HIGHLIGHT_FG_COLOR, HIGHLIGHT_BG_COLOR);
    con_output_stringf(ConOut, u"%s", menu_choices[0]);

    con_set_color(ConOut, DEFAULT_FG_COLOR, DEFAULT_BG_COLOR);
    for (UINTN i = 1; i < ARRAY_SIZE(menu_choices); i++)
    {
      con_output_stringf(ConOut, u"\r\n%s", menu_choices[i]);
    }

    INTN min_row = 0, max_row = con_get_mode_info(ConOut).CursorRow;
    // INTN min_row = 0, max_row = ConOut->Mode->CursorRow;

    ConOut->SetCursorPosition(ConOut, 0, 0);
    bool getting_input = true;
    while (getting_input)
    {
      EFI_INPUT_KEY key = con_get_key(ConIn, BootServices);
      INTN current_row = con_get_mode_info(ConOut).CursorRow;
      // INTN current_row = ConOut->Mode->CursorRow;

      switch (key.ScanCode)
      {
      case SCANCODE_UP_ARROW:
        if (current_row - 1 >= min_row)
        {
          con_set_color(ConOut, DEFAULT_FG_COLOR, DEFAULT_BG_COLOR);
          con_output_stringf(ConOut, u"%s\r", menu_choices[current_row]);

          current_row--;
          ConOut->SetCursorPosition(ConOut, 0, current_row);
          con_set_color(ConOut, HIGHLIGHT_FG_COLOR, HIGHLIGHT_BG_COLOR);
          con_output_stringf(ConOut, u"%s\r", menu_choices[current_row]);
        }
        break;
      case SCANCODE_DOWN_ARROW:
        if (current_row + 1 <= max_row)
        {
          con_set_color(ConOut, DEFAULT_FG_COLOR, DEFAULT_BG_COLOR);
          con_output_stringf(ConOut, u"%s\r", menu_choices[current_row]);

          current_row++;
          ConOut->SetCursorPosition(ConOut, 0, current_row);
          con_set_color(ConOut, HIGHLIGHT_FG_COLOR, HIGHLIGHT_BG_COLOR);
          con_output_stringf(ConOut, u"%s\r", menu_choices[current_row]);
        }
        break;
      case SCANCODE_ESC:
        RuntimeServices->ResetSystem(EfiResetShutdown, EFI_SUCCESS, 0, NULL);
        // system_shutdown(&RuntimeServices);
        __attribute__((noreturn));
        break;
      default:
        if (key.UnicodeChar == u'\r')
        {
          // Enter key
          EFI_STATUS return_status = menu_funcs[current_row]();
          if (EFI_ERROR(return_status))
          {
            con_output_stringf(ConOut, u"ERROR: %x\r\n\r\nPress any key to go back...\r\n", return_status);
            con_get_key(ConOut, BootServices);
          }
          getting_input = false;
        }
        break;
      }
    }
  }
  return EFI_SUCCESS;
}
