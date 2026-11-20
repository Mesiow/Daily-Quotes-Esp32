// Display.ino - e-paper output for the Waveshare 2.9" (B) V4 panel (296x128, black/red/white)
//
// Layout (landscape):
// red accent bar on the left, quote in black (word-wrapped), "- Author" in red bottom-right.

// Screen size after ROTATE_270 (panel is natively 128 wide x 296 tall)
static const int SCREEN_W = 296;
static const int SCREEN_H = 128;
static const int MARGIN   = 8;

// One bit per pixel: 128/8 bytes per row * 296 rows = 4736 bytes per layer
static const UWORD IMAGE_SIZE =
    ((EPD_2IN9B_V4_WIDTH % 8 == 0) ? (EPD_2IN9B_V4_WIDTH / 8) : (EPD_2IN9B_V4_WIDTH / 8 + 1)) * EPD_2IN9B_V4_HEIGHT;

// Static buffers: allocated once, can't fail like malloc can
static UBYTE black_image[IMAGE_SIZE];
static UBYTE red_image[IMAGE_SIZE];

// Call once per boot
void init_display() {
  Serial.println("Init display");
  if (DEV_Module_Init() != 0) {
    Serial.println("DEV_Module_Init failed");
    while (1) delay(1000);
  }
}

// Draws one line of text in "ink" (BLACK) on a transparent background,
// into whichever buffer is currently selected. In the red buffer, BLACK ink shows as red.
// Note: Paint_DrawString_EN swaps its color arguments internally (GUI_Paint.cpp),
// so passing (WHITE, BLACK) here is what produces ink-colored text.
void draw_text(int x, int y, const String &text, sFONT *font) {
  Paint_DrawString_EN(x, y, text.c_str(), font, WHITE, BLACK);
}

// Greedy word wrap: draws text starting at (x, y), breaking lines between words
// so each line fits in max_width pixels. Stops before going past max_y.
// Returns the number of lines drawn.
int draw_wrapped(int x, int y, const String &text, sFONT *font, int max_width, int max_y) {
  const int max_chars = max_width / font->Width;
  String line;
  int line_count = 0;
  int start = 0;
  const int len = text.length();

  while (start < len) {
    int space = text.indexOf(' ', start);
    if (space < 0) space = len;
    String word = text.substring(start, space);
    start = space + 1;

    // Word doesn't fit on the current line: draw the line and start a new one
    if (line.length() > 0 && (int)(line.length() + 1 + word.length()) > max_chars) {
      int line_y = y + line_count * font->Height;
      if (line_y + font->Height > max_y) return line_count;   // out of room
      draw_text(x, line_y, line, font);
      line_count++;
      line = "";
    }
    if (line.length() > 0) line += ' ';
    line += word;
  }

  // Last line
  if (line.length() > 0) {
    int line_y = y + line_count * font->Height;
    if (line_y + font->Height <= max_y) {
      draw_text(x, line_y, line, font);
      line_count++;
    }
  }
  return line_count;
}

// Renders a quote and does one full refresh (~15s), then puts the panel to sleep.
void display_quote(const Quote &quote) {
  sFONT *quote_font  = &Font16;
  sFONT *author_font = &Font12;

  const int bar_w    = 3;
  const int text_x   = MARGIN + bar_w + 8;                         // right of the accent bar
  const int text_w   = SCREEN_W - text_x - MARGIN;
  const int author_y = SCREEN_H - MARGIN - author_font->Height;    // bottom line

  // ---- Black layer: the quote ----
  Paint_NewImage(black_image, EPD_2IN9B_V4_WIDTH, EPD_2IN9B_V4_HEIGHT, ROTATE_270, WHITE);
  Paint_Clear(WHITE);
  int lines = draw_wrapped(text_x, MARGIN, quote.quote, quote_font, text_w, author_y - 4);
  Serial.printf("Quote drawn in %d lines\n", lines);

  // ---- Red layer: accent bar + author ----
  Paint_NewImage(red_image, EPD_2IN9B_V4_WIDTH, EPD_2IN9B_V4_HEIGHT, ROTATE_270, WHITE);
  Paint_Clear(WHITE);
  Paint_DrawRectangle(MARGIN, MARGIN, MARGIN + bar_w, SCREEN_H - MARGIN, RED, DOT_PIXEL_1X1, DRAW_FILL_FULL);

  String author = "- " + quote.author;
  int author_x = SCREEN_W - MARGIN - (int)(author.length() * author_font->Width);  // right-aligned
  if (author_x < text_x) author_x = text_x;                                       // very long names
  draw_text(author_x, author_y, author, author_font);

  // ---- Push both layers to the panel ----
  Serial.println("Refreshing display...");
  EPD_2IN9B_V4_Init();                         // wake the panel
  EPD_2IN9B_V4_Display(black_image, red_image);
  EPD_2IN9B_V4_Sleep();                        // always sleep after drawing
  Serial.println("Display done, panel asleep");
}
