<?php
/**
 * ----------------------------------------------
 * Daily Counter 1.1
 * Copyright (c)2001 Chi Kien Uong
 * URL: http://www.proxy2.de
 * ----------------------------------------------
 */

class dcounter {

    var $config;
    var $ip;
    var $host;
    var $table;
    var $counter;
    var $use_db;
    var $visitor;
    var $db;

    function dcounter() {
        global $COUNT_TBL,$COUNT_CFG;
        if ($COUNT_CFG["use_db"]) {
            $this->db = new dcounter_sql();
            $this->db->connect();
            $this->table = $COUNT_TBL;
        }
        $this->ip = getenv("REMOTE_ADDR");
        $this->host = gethostbyaddr($this->ip);
        $this->config = $COUNT_CFG;
        $this->use_db = ($COUNT_CFG['use_db']) ? true : false;
    }

    function is_new_visitor() {
        $now = $this->config['offset']*3600 + time() - $this->config['block_time'];
        if ($this->use_db) {
            $this->db->query("DELETE FROM ".$this->table['visitors']." WHERE $now > time");
            $this->db->fetch_array($this->db->query("SELECT * FROM ".$this->table['visitors']." WHERE ip = '$this->ip'"));
            if ($this->db->record) {
                $now = $this->config['offset']*3600 + time();
                $this->db->query("UPDATE ".$this->table['visitors']." SET time='$now' WHERE ip = '$this->ip'");
                return false;
            } else {
                return true;
            }
        } else {
            $found=0;
            $rows = @file($this->config['logfile']);
            $this_time = $this->config['offset']*3600 + time();
            $reload_dat = fopen($this->config['logfile'],"wb");
            flock($reload_dat, 2);
            for ($i=0; $i<sizeof($rows); $i++) {
                list($time_stamp,$ip_addr,$hostname) = preg_split("/|/",$rows[$i]);
                if ($this_time < ($time_stamp+$this->config['block_time'])) {
                    if ($ip_addr == $this->ip) {
                        $found=1;
                    } else {
                        fwrite($reload_dat,"$time_stamp|$ip_addr|$hostname");
                    }
                }
            }
            $new = "$this_time|".$this->ip."|".$this->host."\n";
            fwrite($reload_dat,$new);
            flock($reload_dat, 3);
            fclose($reload_dat);
            return ($found==1) ? false : true;
        }
    }

    function update_counter() {
        $now = time()+$this->config['offset']*3600;
        if ($this->use_db) {
            $this->db->query("INSERT INTO ".$this->table['visitors']." (time,ip,host) VALUES ('$now','$this->ip','$this->host')");
            $this->db->query("UPDATE ".$this->table['total']." set total=total+1");
            if ($this->db->affected_rows() == 0) {
                $this->db->query("INSERT INTO ".$this->table['total']." SET total='1', installtime = UNIX_TIMESTAMP()");
            }
            $this->db->query("UPDATE ".$this->table['daily']." SET visitors=visitors+1 WHERE day = CURDATE()");
            if ($this->db->affected_rows() == 0) {
                $this->db->query("INSERT INTO ".$this->table['daily']." SET visitors='1', day = CURDATE()");
            }
        } else {
            $today = date("Y-m-j",time()+$this->config['offset']*3600);
            if (!file_exists($this->config['counter'])) {
                $count_dat = fopen($this->config['counter'],"w+");
                fwrite($count_dat,"1");
                fclose($count_dat);
                $this->counter = 1;
            } else {
                $fp = fopen($this->config['counter'], "r+");
                flock($fp, 2);
                $this->counter = fgets($fp, 4096);
                $this->counter++;
                rewind($fp);
                fwrite($fp, $this->counter);
                flock($fp, 3);
                fclose($fp);
            }
            if (!file_exists($this->config['daylog'])) {
                $count_dat = fopen($this->config['daylog'],"w+");
                fwrite($count_dat,"$today\n1");
                fclose($count_dat);
                $this->visitor = 1;
            } else {
                $fp = fopen($this->config['daylog'], "r+");
                flock($fp, 2);
                $first_row = chop(fgets($fp, 4096));
                $this->visitor = fgets($fp, 4096);
                if ($today == $first_row) {
                    $this->visitor++;
                } else {
                    $this->visitor = 1;
                }
                rewind($fp);
                fwrite($fp,"$today\n");
                fwrite($fp,$this->visitor);
                flock($fp, 3);
                fclose($fp);
            }
        }
    }

    function show_counter() {
        if ($this->use_db) {
            if ($this->is_new_visitor()) {
                $this->update_counter();
            }
            $this->db->fetch_array($this->db->query("SELECT total FROM ".$this->table['total']));
            $counter['total'] = $this->db->record['total'];
            $this->db->fetch_array($this->db->query("SELECT visitors FROM ".$this->table['daily']." WHERE day = CURDATE()"));
            $counter['visits_today'] = $this->db->record['visitors'];
            $this->db->close_db();

        } else {
            if ($this->is_new_visitor()) {
                $this->update_counter();
            } else {
                $fp = fopen($this->config['counter'], "r");
                flock($fp, 2);
                $this->counter = fgets($fp, 4096);
                flock($fp, 3);
                fclose($fp);
                $fp = fopen($this->config['daylog'], "r");
                flock($fp, 2);
                $first_row = fgets($fp, 4096);
                $this->visitor = chop(fgets($fp, 4096));
                flock($fp, 3);
                fclose($fp);
            }
            $counter['total'] = $this->counter;
            $counter['visits_today'] = $this->visitor;
        }
        return $counter;
    }

}

?>
