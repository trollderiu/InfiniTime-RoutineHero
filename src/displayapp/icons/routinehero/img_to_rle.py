from PIL import Image
import json

def decode_rle(encoded_buffer, display_width, bytes_per_pixel):
    output = bytearray()
    color = 0xFF  # Initial color, assuming white
    # color = 0x00  # Initial color, assuming white
    processed_count = 0
    for byte in encoded_buffer:
        rl = byte - processed_count
        while rl:
            output.append(color)
            rl -= 1
            processed_count += 1
            if processed_count >= display_width * bytes_per_pixel:
                processed_count = 0
                output.extend([0xFF] * (display_width - (len(output) // display_width)))
                color = 0xFF if color == 0x00 else 0x00  # Toggle color
                # color = 0xFF if color == 0xFF else 0x00  # Toggle color
        processed_count = 0
        color = 0xFF if color == 0x00 else 0x00  # Toggle color
        # color = 0xFF if color == 0xFF else 0x00  # Toggle color
    return output

def encode_rle(decoded_buffer):
    output = []
    # current_color = None
    current_color = 255

    count = 0
    for color in decoded_buffer:

        if count == 255:
            output.append(count)
            output.append(0)
            count = 0
    
        if current_color is None:
            current_color = color
            count = 1
        elif color == current_color:
            count += 1
        else:
            output.append(count)
            current_color = 0 if current_color == 255 else 255
            count = 1

    # Append the last run
    output.append(current_color)
    output.append(count)

    return output

img = Image.open("decoded_image.png")
decoded_data = img.tobytes()
display_width = img.width
decoded_data = img.convert("L").tobytes()
height = len(decoded_data) // display_width
decoded_data = decoded_data[:display_width * height]

encoded_data = encode_rle(decoded_data)

# Convert each integer to its corresponding 8-bit byte and then to hexadecimal format
hex_bytes = [f'0x{i:02x}' for i in encoded_data]

# Create the formatted string
formatted_string = '{\n    ' + ', '.join(hex_bytes) + ',\n}'

# Write the formatted string to a file
with open('output.txt', 'w') as file:
    file.write(formatted_string)