import lgpio
import time

SPI_BUS = 0
SPI_CS = 0
SPI_SPEED = 500000  # 500 kHz

# Define the command for reading the status
READ_STATUS_CMD = 0xC0  # Replace with the actual command from the datasheet

def send_command(handle, command):
    lgpio.spi_write(handle, [command])

def read_status(handle):
    send_command(handle, READ_STATUS_CMD)
    time.sleep(0.1)  # Wait for the command to process
    # Attempt to read 2 bytes instead of 1 for better status handling
    status, _ = lgpio.spi_read(handle, 2)  # Adjust number of bytes based on datasheet
    return status

def main():
    h = lgpio.spi_open(SPI_BUS, SPI_CS, SPI_SPEED)
    if h < 0:
        print("Failed to open SPI bus")
        return

    # Perform a reset or initialization if necessary
    # Example: send_command(h, INIT_CMD)  # Replace with actual initialization command

    status = read_status(h)
    print(f"Status: {status}")

    lgpio.spi_close(h)

if __name__ == "__main__":
    main()

