# TO RUN

~/spark-2.4.5-bin-hadoop2.7/bin/spark-submit --class "Ex2" --master spark://prometheus.shuffling-bench.wisr-pg0.clemson.cloudlab.us:7077 --conf spark.eventLog.enabled=true --conf spark.eventLog.dir="/users/balarjun/shebin/history-test" target/scala-2.11/ex2_2.11-1.0.jar hdfs://10.10.1.2:9000/cs740/graphx/data/data.txt
