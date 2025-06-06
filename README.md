# He-Dieu-Hanh-Nhung
GVHD Pham Doan Tinh

Về dự án này:
Trình điều khiển thiết bị thực hiện đĩa RAM ảo. Khi mô-đun được tải, người dùng có thể đọc và ghi /dev/asgn1 để tương tác với thiết bị. Thiết bị duy trì danh sách các trang, khi ghi vào thiết bị, các trang mới sẽ tự động được phân bổ theo yêu cầu.

Người dùng có thể sử dụng IOCTL để thiết lập số lượng tối đa các tiến trình có thể truy cập thiết bị. Thông tin gỡ lỗi có thể được xuất ra bằng cách đọc từ /proc/asgn1. Tất cả các trang có thể được giải phóng khi mở thiết bị ở chế độ chỉ ghi.
