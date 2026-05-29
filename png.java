import java.util.zip.*;      // Import the CRC32 class
import java.util.ArrayList;
import java.util.List;
import java.io.*;

public class png {
	public static class chunk {
		public int length;
		public String type;
		public byte[] data;
		public int crc;
	
		public chunk(int length, String type, byte[] data, int crc) {
			this.length = length;
			this.type = type;
			this.data = data;
			this.crc = crc;
			
		}
	}

	private chunk IHDR;
	private chunk PLTE;
	private chunk[] IDAT;
	private final chunk IEND = new chunk(
		0,
		"IEND",
		new byte[0],
		0xAE426082
	);
	// Getter
	public chunk IHDR(){
	return this.IHDR;
	}
	
	public chunk PLTE(){
	return this.PLTE;
	}
	
	public chunk[] IDAT(){
	return this.IDAT;
	}
	
	public chunk IEND(){
	return this.IEND;
	}
	
	public void verifySignature(DataInputStream dis) throws IOException {
	    // 1. Define the expected signature
	    byte[] expected = {(byte) 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
	    
	    // 2. Read the actual first 8 bytes from the file
	    byte[] actual = new byte[8];
	    dis.readFully(actual);
	    
	    // 3. Compare the arrays
	    for (int i = 0; i < 8; i++) {
	        if (actual[i] != expected[i]) {
	            throw new IOException("Invalid PNG signature! This is not a PNG file.");
	        }
	    }
	    System.out.println("Signature verified successfully.");
	}
	
	public static void main(String[] args) {
	    if (args.length > 0) {
	        String filename = args[0];
	        System.out.println("Attempting to parse: " + filename);

	        // Using DataInputStream for convenient reading of 4-byte integers (Big-Endian)
	        try (DataInputStream dis = new DataInputStream(new FileInputStream(filename))) {
	            
	        	verifySignature(dis);
	            
	            List<chunk> idatList = new ArrayList<>();
	            boolean parsing = true;

	            while (parsing) {
	                // 2. Read Chunk Length (4 bytes)
	                int length = dis.readInt();
	                
	                // 3. Read Chunk Type (4 bytes)
	                byte[] typeBytes = new byte[4];
	                dis.readFully(typeBytes);
	                String type = new String(typeBytes);
	                
	                // 4. Read Chunk Data (variable length)
	                byte[] data = new byte[length];
	                dis.readFully(data);
	                
	                // 5. Read CRC (4 bytes)
	                int crc = dis.readInt();
	                
	                System.out.println("Parsed chunk: " + type + " (" + length + " bytes)");

	                // Store in your class (You'd need setter methods for these)
	                if (type.equals("IEND")) {
	                    parsing = false;
	                }
	            }
	        } catch (IOException e) {
	            System.err.println("File error: " + e.getMessage());
	            System.err.println("Header verification failed: " + e.getMessage());
	        }
	    } else {
	        System.out.println("Please provide a PNG file path as an argument.");
	    }
	}
} //this is good start