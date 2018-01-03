package main

import (
	"encoding/hex"
	"image"
	"image/png"
	"log"
	"os"
    "fmt"
    "strings"
)

func process_img(i image.Image, output string) string {
	pages := i.Bounds().Size().Y / 8
	width := i.Bounds().Size().X

	dump := "unsigned char const " + output + "_[] = {"
	for p := 0; p < pages; p++ {
		for x := 0; x < width; x++ {
			var v byte = 0
			for o := 0; o < 8; o++ {
				y := p*8 + 7 - o
				c := i.At(x, y)

				var z byte = 0
				_, _, _, a := c.RGBA()
				if a > 0 {
					z = 1
				}

				v = (v << 1) | z
			}
			dump += "0x"
			dump += hex.EncodeToString([]byte{v})

			if p != (pages-1) || x != (width-1) {
				dump += ", "
			}
		}
	}

	dump += "};"
    return dump;
}

func main() {
    for i := 1; i < len(os.Args); i++ {
        file, err := os.Open(os.Args[i])
        if err != nil {
            log.Fatal(err)
        }

        img, err := png.Decode(file)
        if err != nil {
            log.Fatal(err)
        }

        dump := process_img(img, strings.TrimSuffix(os.Args[i], ".png"))

        fmt.Println(dump)
        file.Close()
    }
}
