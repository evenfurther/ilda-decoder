#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <SDL2/SDL.h>

#include "ilda_decoder.h"

static ssize_t read_file(void *opaque, void *buffer, size_t len) {
  return read((int)(long)opaque, buffer, len);
}

#define DIM 600

int transform(int x) { return (x + 32768) * 599 / 65535; }

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Usage: display_ilda FILE\n");
    exit(1);
  }
  int f = open(argv[1], 0);
  if (f < 0) {
    perror("cannot open file");
    exit(1);
  }
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    fprintf(stderr, "cannot initialize SDL\n");
    exit(1);
  }
  SDL_Window *window =
      SDL_CreateWindow("ILDA display", SDL_WINDOWPOS_CENTERED,
                       SDL_WINDOWPOS_CENTERED, DIM, DIM, SDL_WINDOW_SHOWN);
  if (!window) {
    fprintf(stderr, "cannot create SDL window\n");
    exit(1);
  }
  SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);
  for (;;) {
    ilda_state_t ilda;
    ilda_init(&ilda, read_file, (void *)(long)f);
    ilda_pos_t last_point = {0, 0, 0};
    for (;;) {
      SDL_Event event;
      while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT)
          goto bye;
      }
      const ilda_header_t *header = ilda_read_next_header(&ilda);
      if (header == NULL) {
        fprintf(stderr, "error when reading ILDA file: %s\n", ilda.error);
        exit(1);
      }
      if (ilda_is_end_of_file(header)) {
        break;
      }
      if (ilda_is_palette(header)) {
        if (ilda_read_palette(&ilda)) {
          fprintf(stderr, "error when reading palette: %s\n", ilda.error);
          exit(1);
        }
      } else {
        static ilda_point_t buffer[65536];
        if (ilda_read_records(&ilda, buffer, sizeof buffer)) {
          fprintf(stderr, "error when reading records: %s\n", ilda.error);
          exit(1);
        }
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(renderer);
        for (size_t i = 0; i < header->number_of_records; i++) {
          const ilda_point_t *point = &buffer[i];
          if (!ilda_is_blanking(point->status_code)) {
            SDL_SetRenderDrawColor(renderer, point->color.r, point->color.g,
                point->color.b, SDL_ALPHA_OPAQUE);
            SDL_RenderDrawLine(renderer, transform(last_point.x),
                DIM - transform(last_point.y), transform(point->pos.x),
                DIM - transform(point->pos.y));
          }
          memcpy(&last_point, point, sizeof last_point);
        }
        SDL_RenderPresent(renderer);
        SDL_Delay(40);
      }
    }
    lseek(f, 0, SEEK_SET);
  }

bye:
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
}
