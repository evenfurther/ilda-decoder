#include <string.h>

#include "ilda-decoder.h"

static const ilda_color_t default_color_palette[64] = {
    {255, 0, 0},     {255, 16, 0},    {255, 32, 0},    {255, 48, 0},
    {255, 64, 0},    {255, 80, 0},    {255, 96, 0},    {255, 112, 0},
    {255, 128, 0},   {255, 144, 0},   {255, 160, 0},   {255, 176, 0},
    {255, 192, 0},   {255, 208, 0},   {255, 224, 0},   {255, 240, 0},
    {255, 255, 0},   {224, 255, 0},   {192, 255, 0},   {160, 255, 0},
    {128, 255, 0},   {96, 255, 0},    {64, 255, 0},    {32, 255, 0},
    {0, 255, 0},     {0, 255, 36},    {0, 255, 73},    {0, 255, 109},
    {0, 255, 146},   {0, 255, 182},   {0, 255, 219},   {0, 255, 255},
    {0, 227, 255},   {0, 198, 255},   {0, 170, 255},   {0, 142, 255},
    {0, 113, 255},   {0, 85, 255},    {0, 56, 255},    {0, 28, 255},
    {0, 0, 255},     {32, 0, 255},    {64, 0, 255},    {96, 0, 255},
    {128, 0, 255},   {160, 0, 255},   {192, 0, 255},   {224, 0, 255},
    {255, 0, 255},   {255, 32, 255},  {255, 64, 255},  {255, 96, 255},
    {255, 128, 255}, {255, 160, 255}, {255, 192, 255}, {255, 224, 255},
    {255, 255, 255}, {255, 224, 224}, {255, 192, 192}, {255, 160, 160},
    {255, 128, 128}, {255, 96, 96},   {255, 64, 64},   {255, 32, 32}};

void ilda_init(ilda_state_t *ilda, ssize_t (*read)(void *, void *, size_t),
               void *opaque, int strict_mode) {
  memset(ilda, 0, sizeof *ilda);
  ilda->provider.read = read;
  ilda->provider.opaque = opaque;
  ilda->strict_mode = strict_mode != 0;
  memcpy(ilda->palette, default_color_palette, sizeof default_color_palette);
}

static const char *replenish(ilda_provider_t *provider, uint8_t *buffer,
                             size_t len) {
  size_t missing = len;
  while (missing) {
    ssize_t bytes = provider->read(provider->opaque, buffer, missing);
    if (bytes == 0)
      return "end-of-file while replenishing ";
    if (bytes < 0)
      return "error while replenishing ";
    if (bytes > (ssize_t)missing)
      return "too many bytes read, possible data corruption";
    missing -= bytes;
  }
  return NULL;
}

const ilda_header_t *ilda_read_next_header(ilda_state_t *ilda) {
  if (ilda->error)
    return NULL;
  static uint8_t buffer[32];
  ilda->error = replenish(&ilda->provider, buffer, sizeof buffer);
  if (ilda->error)
    return NULL;
  if (buffer[0] != 'I' || buffer[1] != 'L' || buffer[2] != 'D' ||
      buffer[3] != 'A') {
    ilda->error = "unable to find ILDA marker in header";
    return NULL;
  }
  const uint8_t format_code = buffer[7];
  if (format_code > 5 || format_code == 3) {
    ilda->error = "invalid format code";
    return NULL;
  }
  memset(&ilda->current_header, 0, sizeof ilda->current_header);
  ilda->current_header.format_code = format_code;
  memcpy(ilda->current_header.frame_or_color_palette_name, (char *)&buffer[8],
         8);
  memcpy(ilda->current_header.company_name, (char *)&buffer[16], 8);
  const uint16_t number_of_records = (buffer[24] << 8) | buffer[25];
  if (format_code == 2 && number_of_records > 256) {
    ilda->error = "too many records for palette";
    return NULL;
  }
  ilda->current_header.number_of_records = number_of_records;
  ilda->current_header.frame_or_color_palette_number =
      (buffer[26] << 8) | buffer[27];
  ilda->current_header.total_frames = (buffer[28] << 8) | buffer[29];
  if (ilda->strict_mode) {
    if (format_code == 2 && number_of_records < 2) {
      ilda->error = "too few records for palette";
      return NULL;
    }
    if (ilda->current_header.frame_or_color_palette_number == 65535) {
      ilda->error = "invalid frame or color palette number";
      return NULL;
    }
    if (format_code == 2 && ilda->current_header.total_frames) {
      ilda->error = "number of frames in color palette should be 0";
      return NULL;
    }
    if (ilda->current_header.total_frames == 0) {
      ilda->error = "invalid number of frames in sequence";
      return NULL;
    }
  }
  ilda->current_header.projector_number = buffer[30];
  return &ilda->current_header;
}

int ilda_is_palette(const ilda_header_t *header) {
  return header->format_code == 2;
}

int ilda_read_palette(ilda_state_t *ilda) {
  static uint8_t buffer[256 * 3];
  if (ilda->error) {
    return 1;
  }
  if (!ilda_is_palette(&ilda->current_header)) {
    ilda->error = "cannot read palette from a non-palette section";
    return 1;
  }
  const uint16_t nor = ilda->current_header.number_of_records;
  ilda->error = replenish(&ilda->provider, buffer, nor * 3);
  if (ilda->error)
    return 1;
  for (size_t i = 0; i < nor; i++) {
    ilda->palette[i].r = buffer[3 * i];
    ilda->palette[i].g = buffer[3 * i + 1];
    ilda->palette[i].b = buffer[3 * i + 2];
  }
  return 0;
}

int ilda_read_records(ilda_state_t *ilda, ilda_point_t *points, size_t len) {
  if (ilda->error)
    return 1;
  if (ilda_is_palette(&ilda->current_header)) {
    ilda->error = "cannot read records from a palette section";
    return 1;
  }
  const uint16_t nor = ilda->current_header.number_of_records;
  if (len < nor * sizeof(ilda_point_t)) {
    ilda->error = "too small a buffer to decode records";
    return 1;
  }
  const uint8_t format_code = ilda->current_header.format_code;
  static const size_t records_size[] = {8, 6, 0, 0, 10, 8};
  const size_t record_size = records_size[format_code];
  for (size_t i = 0; i < nor; i++) {
    static uint8_t buffer[10];
    ilda->error = replenish(&ilda->provider, buffer, record_size);
    if (ilda->error)
      return 1;
    points[i].pos.x = (buffer[0] << 8) | buffer[1];
    points[i].pos.y = (buffer[2] << 8) | buffer[3];
    switch (format_code) {
    case 0:
      points[i].pos.z = (buffer[4] << 8) | buffer[5];
      memcpy(&points[i].color, &ilda->palette[buffer[7]], 3);
      points[i].status_code = buffer[6];
      break;
    case 1:
      points[i].pos.z = 0;
      memcpy(&points[i].color, &ilda->palette[buffer[5]], 3);
      points[i].status_code = buffer[4];
      break;
    case 4:
      points[i].pos.z = (buffer[4] << 8) | buffer[5];
      points[i].color.r = buffer[9];
      points[i].color.g = buffer[8];
      points[i].color.b = buffer[7];
      points[i].status_code = buffer[6];
      break;
    case 5:
      points[i].pos.z = 0;
      points[i].color.r = buffer[7];
      points[i].color.g = buffer[6];
      points[i].color.b = buffer[5];
      points[i].status_code = buffer[4];
      break;
    }
  }
  return 0;
}

int ilda_is_end_of_file(const ilda_header_t *header) {
  return !ilda_is_palette(header) && header->number_of_records == 0;
}
