using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;

namespace TemplePlusConfig
{
    /// <summary>
    /// Interaction logic for HouseRulesWindow.xaml
    /// </summary>
    public partial class HouseRulesWindow : Window
    {
        public HouseRulesWindow()
        {
            InitializeComponent();

            DataContext = App._iniViewModel;
        }

        private void button_Click(object sender, RoutedEventArgs e){
            this.Close();
            
        }

        private void CheckBox_Checked(object sender, RoutedEventArgs e)
        {
            this.Width = 670;
        }

        private void CheckBox_Unchecked(object sender, RoutedEventArgs e)
        {
            this.Width = 438;
        }

        private void CheckBox_Initialized(object sender, EventArgs e)
        {

            var chkbx = sender as CheckBox;
           if (chkbx != null && chkbx.IsChecked != null){
                if (chkbx.IsChecked == true){
                    CheckBox_Checked(sender, null);
                }
                else
                {
                    CheckBox_Unchecked(sender, null);
                }
            }
            
        }

        private void CheckBox_Checked_1(object sender, RoutedEventArgs e)
        {

        }
    }
}
