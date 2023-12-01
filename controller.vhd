library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity mouse_controller is
	generic (
		SOC_SEGMENT : integer := 	16#e8#;		-- SoC segment address is 0xe8
		SOC_CLASS : integer := 		16#00#		-- Class address is 0x00
	);
	port (
		-- CPU interface signals
		clk_i : in std_logic;
		rst_i : in std_logic;
		ps2_clk : inout std_logic;
		ps2_data : inout std_logic;
		addr_i : in std_logic_vector(31 downto 0);
		data_o : out std_logic_vector(31 downto 0);
		data_access_o : out std_logic
	);
end mouse_controller;

architecture mouse_controller_arch of mouse_controller is
	signal reset_n,data_access,enable_read: std_logic;
    signal device : std_logic_vector(15 downto 0);
	signal data_read: std_logic_vector(31 downto 0);
	signal data_out: std_logic_vector(23 downto 0);
begin
	device <= addr_i(15 downto 0);
	data_access <= '1' when addr_i(31 downto 24) = std_logic_vector(to_unsigned(SOC_SEGMENT, 8)) and
				addr_i(23 downto 16) = std_logic_vector(to_unsigned(SOC_CLASS, 8))
				else '0';
	data_access_o <= data_access;
	data_o <= data_read;
	
    reset_n <= not(rst_i);

    mouse: entity work.ps2_mouse
    port map( 
        clk => clk_i,        
        reset_n => reset_n,
        ps2_clk => ps2_clk,     
        ps2_data => ps2_data,     
        mouse_data => data_out,
        mouse_data_new => enable_read 
    );

	process(clk_i, rst_i, device)
	begin
		if rst_i = '1' then
			data_read <= (others => '0');
		elsif rising_edge(clk_i) then
			if (data_access = '1') then
				case device(6 downto 4) is
					when "001" => data_read <= x"0000000" & "000" & enable_read;
					when "010" => data_read <= x"00" & data_out;
					when others => data_read <= (others => '0');
				end case;
			end if;
		end if;
	end process;
		
end mouse_controller_arch;