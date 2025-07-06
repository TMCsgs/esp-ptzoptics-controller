import time
import requests
import serial


SERIAL_PORT = 'COM3'
BAUD_RATE = 9600
def main(): 
    try:
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
        #print(f"Überwachung gestartet auf {SERIAL_PORT} mit {BAUD_RATE} Baud.")
    except serial.SerialException as e:
        #print(f"Fehler beim Öffnen des seriellen Ports: {e}")
        return

    while True:
        try:
            if ser.in_waiting > 0:
                # Daten vom Arduino lesen
                line = ser.readline().decode('utf-8').strip()
                if line:
                    #print(f"Empfangene URL: {line}")
                    # URL an den Webserver senden
                    #try:
                        response = requests.post(line)
                        #if response.status_code == 200:
                        #    print("URL erfolgreich aufgerufen.")
                        #else:
                        #    print(f"Fehler beim Weiterleiten: Status {response.status_code}")
                    #except requests.RequestException as e:
                    #    print(f"Fehler beim Senden der Anfrage: {e}")
            time.sleep(0.1)
        except KeyboardInterrupt:
            #print("Beende Überwachung.")
            break
    ser.close()

if __name__ == "__main__": main()

