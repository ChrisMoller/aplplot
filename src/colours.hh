#pragma once

typedef struct {
  const char *name;
  unsigned char red;
  unsigned char green;
  unsigned char blue;
} colours_s;

colours_s *colour_lookup (const char *col);

