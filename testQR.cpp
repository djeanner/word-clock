

#include "thirdParty/qrcodegen.hpp"
#include <climits>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using qrcodegen::QrCode;
using qrcodegen::QrSegment;
using std::uint8_t;

// Function prototypes
static void doBasicDemo();
static std::string toSvgString(const QrCode &qr, int border);
static void printQr(const QrCode &qr);

// The main application program.
int main() {
  doBasicDemo();
  return EXIT_SUCCESS;
}

/*---- Demo suite ----*/

// Creates a single QR Code, then prints it to the console.
static void doBasicDemo() {
#include "hiddenFile.h"
  // const char *text = "Oui!";              // User-supplied text
  const QrCode::Ecc errCorLvl = QrCode::Ecc::LOW; // Error correction level

  // Make and print the QR Code symbol
  const QrCode qr = QrCode::encodeText(textP, errCorLvl);
  printQr(qr);
  std::cout << toSvgString(qr, 4) << std::endl;
}

/*---- Utilities ----*/

// Returns a string of SVG code for an image depicting the given QR Code, with
// the given number of border modules. The string always uses Unix newlines
// (\n), regardless of the platform.
static std::string toSvgString(const QrCode &qr, int border) {

  std::ostringstream sb;
  sb << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
  sb << "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" "
        "\"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n";
  sb << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" viewBox=\"0 "
        "0 ";
  sb << (qr.getSize() + border * 2) << " " << (qr.getSize() + border * 2)
     << "\" stroke=\"none\">\n";
  sb << "\t<rect width=\"100%\" height=\"100%\" fill=\"#FFFFFF\"/>\n";
  sb << "\t<path d=\"";
  for (int y = 0; y < qr.getSize(); y++) {
    for (int x = 0; x < qr.getSize(); x++) {
      if (qr.getModule(x, y)) {
        if (x != 0 || y != 0)
          sb << " ";
        sb << "M" << (x + border) << "," << (y + border) << "h1v1h-1z";
      }
    }
  }
  sb << "\" fill=\"#000000\"/>\n";
  sb << "</svg>\n";
  return sb.str();
}

// Prints the given QrCode object to the console.
static void printQr(const QrCode &qr) {
  int border = 4; // required by QR standard !
  for (int y = -border; y < qr.getSize() + border; y++) {
    for (int x = -border; x < qr.getSize() + border; x++) {
      std::cout << (qr.getModule(x, y) ? "##" : "  ");
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;
}
