// =============================================
// Made by QAEZZ A/K/A Reinitialized.
// Copyright (c) 2024 QAEZZ A/K/A Reinitialized
//
// License: MIT
//
// - Implemented -------------------------------
//  Console Output
//  Console Input
//  Console Info
//
// - To-do -------------------------------------
//  N/A
//
// =============================================

#ifndef EFI_HELPERS_H
#define EFI_HELPERS_H

#include <stdarg.h>
#include <stdint.h>
#include "efi.h"

// =============================================
// Console Output
// - Description -------------------------------
//  A few function helpers to provide the dev
//  with simple ways of using ConOut.
// =============================================

EFI_STATUS con_output_string(EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *ConOut, CHAR16 *message)
{
  if (ConOut == NULL || message == NULL)
  {
    return EFI_INVALID_PARAMETER;
  }
  return ConOut->OutputString(ConOut, message);
}

// con_output_hex adapted from Queso Fuego's "UEFI Dev (in C)" Episode 9.
bool con_output_hex(EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *ConOut, UINTN number) {
  const CHAR16 *digits = u"0123456789ABCDEF";
  CHAR16 buffer[20]; // enough for an UINTN_MAX, hopefully
  UINTN i = 0;

  do
  {
    buffer[i++] = digits[number % 16];
    number /= 16;
  } while (number > 0);

  buffer[i++] = u'x';
  buffer[i++] = u'0';

  // 123 -> buffer = 321
  buffer[i--] = u'\0'; // NULL terminate the string

  // reverse digits in buffer
  for (UINTN j = 0; j < i; j++, i--)
  {
    UINTN temp = buffer[i];
    buffer[i] = buffer[j];
    buffer[j] = temp;
  }

  con_output_string(ConOut, buffer);

  return true;
}

// con_output_int32 adapted from Queso Fuego's "UEFI Dev (in C)" Episode 9.
bool con_output_int32(EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *ConOut, INT32 number)
{
  const CHAR16 *digits = u"0123456789";
  CHAR16 buffer[11]; // enough for an INT32_MAX + sign character
  UINTN i = 0;
  const bool negative = (number < 0);

  if (negative) number = -number;

  do
  {
    buffer[i++] = digits[number % 10];
    number /= 10;
  } while (number > 0);

  if (negative)
  {
    buffer[i++] = u'-';
  }

  // 123 -> buffer = 321
  buffer[i--] = u'\0'; // NULL terminate the string

  // reverse digits in buffer
  for (UINTN j = 0; j < i; j++, i--)
  {
    UINTN temp = buffer[i];
    buffer[i] = buffer[j];
    buffer[j] = temp;
  }

  con_output_string(ConOut, buffer);

  return true;
}

// con_output_stringf adapted from Queso Feugo's "UEFI Dev (in C)" Episode 9.
bool con_output_stringf(EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *ConOut, CHAR16 *format, ...)
{
  bool result = false;
  CHAR16 charstr[2] = { u'\0', u'\0' }; // Initialize with null characters

  va_list args;
  va_start(args, format);

  for (UINTN i = 0; format[i] != u'\0'; i++)
  {
    if (format[i] == u'%')
    {
      i++;

      // go to next arg and print it
      switch (format[i])
      {
      case u's': // print CHAR16
      {
        CHAR16 *string = va_arg(args, CHAR16*);
        con_output_string(ConOut, string);
      }
      break;

      case u'd': // print INT32
      {
        INT32 number = va_arg(args, INT32);
        con_output_int32(ConOut, number);
      }
      break;

      case u'x': // print hex UINTN
      {
        UINTN number = va_arg(args, UINTN);
        con_output_hex(ConOut, number);
      }
      break;

      default:
        con_output_string(ConOut, u"Invalid format specifier: %");
        charstr[0] = format[i];
        con_output_string(ConOut, charstr);
        con_output_string(ConOut, u"\r\n");
        result = false;
        goto end;
        break;
      }
    }
    else
    {
      // not formatted string, print next character
      charstr[0] = format[i];
      con_output_string(ConOut, charstr);
    }
  }
end:
  va_end(args);
  result = true;
  return result;
}


EFI_STATUS con_reset_output(EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *ConOut)
{
  if (ConOut == NULL)
  {
    return EFI_INVALID_PARAMETER;
  }
  return ConOut->Reset(ConOut, FALSE);
}

EFI_STATUS con_clear_screen(EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *ConOut)
{
  if (ConOut == NULL)
  {
    return EFI_INVALID_PARAMETER;
  }
  return ConOut->ClearScreen(ConOut);
}

EFI_STATUS con_set_color(EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *ConOut, UINTN fg, UINTN bg)
{
  if (ConOut == NULL)
  {
    return EFI_INVALID_PARAMETER;
  }
  return ConOut->SetAttribute(ConOut, EFI_TEXT_ATTR(fg, bg));
}

// =============================================
// Console Input
// - Description -------------------------------
//  NULL, lmao.
// =============================================

// con_get_key adapted from Queso Feugo's "UEFI Dev (in C)" Episode 10.
EFI_INPUT_KEY con_get_key(EFI_SIMPLE_TEXT_INPUT_PROTOCOL *ConIn, EFI_BOOT_SERVICES *BootServices) {
  EFI_EVENT events[1];
  EFI_INPUT_KEY key;

  key.ScanCode = 0;
  key.UnicodeChar = u'\0';

  events[0] = ConIn->WaitForKey;
  UINTN index = 0;
  BootServices->WaitForEvent(1, events, &index);
  if (index == 0) {
    ConIn->ReadKeyStroke(ConIn, &key);
  }

  return key;
}


// =============================================
// Console Info
// - Description -------------------------------
//  NULL, lmao.
// =============================================

typedef struct {
  UINTN cols;
  UINTN rows;
} ConDimensions;

ConDimensions con_get_query_dimensions(EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *ConOut) {
  ConDimensions dimensions = {0,0};

  ConOut->QueryMode(ConOut, ConOut->Mode->Mode, &dimensions.cols, &dimensions.rows);

  return dimensions;
}

VOID con_update_query_dimensions(EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *ConOut, UINTN *cols, UINTN *rows) {

  ConOut->QueryMode(ConOut, ConOut->Mode->Mode, &cols, &rows);
}

SIMPLE_TEXT_OUTPUT_MODE con_get_mode_info(EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *ConOut) {
  SIMPLE_TEXT_OUTPUT_MODE info = {
    ConOut->Mode->MaxMode,
    ConOut->Mode->Attribute,
    ConOut->Mode->CursorColumn,
    ConOut->Mode->CursorRow,
    ConOut->Mode->CursorVisible
  };
  return info;
}

// void con_update_mode_info(EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *ConOut, SIMPLE_TEXT_OUTPUT_MODE *mode_info) {
//   SIMPLE_TEXT_OUTPUT_MODE info = con_get_mode_info(ConOut);
//   mode_info->MaxMode = info.MaxMode;
//   mode_info->Attribute = info.Attribute;
//   mode_info->CursorColumn = info.CursorColumn;
//   mode_info->CursorRow = info.CursorRow;
//   mode_info->CursorVisible = info.CursorVisible;
// }

#endif // EFI_HELPERS_H